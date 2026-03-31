/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_tpu
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_TPU_H__
#define __REG_TPU_H__

typedef union {
    struct {
        unsigned int mem_ecc_1bit_int_en : 1; /* [0] */
        unsigned int mem_ecc_2bit_int_en : 1; /* [1] */
        unsigned int fifo_rd_int_en : 1;      /* [2] */
        unsigned int fifo_wr_int_en : 1;      /* [3] */
        unsigned int fsm_hbeat_int_en : 1;    /* [4] */
        unsigned int axi_err_int_en : 1;      /* [5] */
        unsigned int rsv_0 : 26;              /* [31:6] */
    } bits;

    unsigned int value;
} u_tpu_int_enable;

typedef union {
    struct {
        unsigned int mem_ecc_1bit_int_sts : 1; /* [0] */
        unsigned int mem_ecc_2bit_int_sts : 1; /* [1] */
        unsigned int fifo_rd_int_sts : 1;      /* [2] */
        unsigned int fifo_wr_int_sts : 1;      /* [3] */
        unsigned int fsm_hbeat_int_sts : 1;    /* [4] */
        unsigned int axi_err_int_sts : 1;      /* [5] */
        unsigned int rsv_1 : 26;               /* [31:6] */
    } bits;

    unsigned int value;
} u_tpu_int_status;

typedef union {
    struct {
        unsigned int mem_ecc_1bit_int_set : 1; /* [0] */
        unsigned int mem_ecc_2bit_int_set : 1; /* [1] */
        unsigned int fifo_rd_int_set : 1;      /* [2] */
        unsigned int fifo_wr_int_set : 1;      /* [3] */
        unsigned int fsm_hbeat_int_set : 1;    /* [4] */
        unsigned int axi_err_int_set : 1;      /* [5] */
        unsigned int rsv_2 : 26;               /* [31:6] */
    } bits;

    unsigned int value;
} u_tpu_int_set;

typedef union {
    struct {
        unsigned int mem_ecc_1bit_int_src : 1; /* [0] */
        unsigned int mem_ecc_2bit_int_src : 1; /* [1] */
        unsigned int fifo_rd_int_src : 1;      /* [2] */
        unsigned int fifo_wr_int_src : 1;      /* [3] */
        unsigned int fsm_hbeat_int_src : 1;    /* [4] */
        unsigned int axi_err_int_src : 1;      /* [5] */
        unsigned int rsv_3 : 26;               /* [31:6] */
    } bits;

    unsigned int value;
} u_tpu_int_source;

typedef union {
    struct {
        unsigned int pkt_dma_mem0_ecc_req : 2;   /* [1:0] */
        unsigned int rsv_4 : 6;                  /* [7:2] */
        unsigned int l3cks_pld_mem0_ecc_req : 2; /* [9:8] */
        unsigned int rsv_5 : 6;                  /* [15:10] */
        unsigned int prebd_mem0_ecc_req : 2;     /* [17:16] */
        unsigned int rsv_6 : 14;                 /* [31:18] */
    } bits;

    unsigned int value;
} u_tpu_mem_err_insert1;

typedef union {
    struct {
        unsigned int ecc_mbit_err_addr : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tpu_mem_err_info;

typedef union {
    struct {
        unsigned int ecc_sbit_err_addr : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tpu_mem_err_info1;

typedef union {
    struct {
        unsigned int axi_rw_int_info : 64; /* [63:0] */
    } bits;

    unsigned int value;
} u_tpu_tx_axi_err_info;

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
} u_tpu_fifo_wr_int0;

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
} u_tpu_fifo_wr_int1;

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
} u_tpu_fifo_wr_int2;

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
} u_tpu_fifo_wr_int3;

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
} u_tpu_fifo_wr_int4;

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
} u_tpu_fifo_wr_int5;

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
} u_tpu_fifo_wr_int6;

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
} u_tpu_fifo_wr_int7;

typedef union {
    struct {
        unsigned int fifo_wr_int256 : 1; /* [0] */
        unsigned int fifo_wr_int257 : 1; /* [1] */
        unsigned int fifo_wr_int258 : 1; /* [2] */
        unsigned int fifo_wr_int259 : 1; /* [3] */
        unsigned int fifo_wr_int260 : 1; /* [4] */
        unsigned int fifo_wr_int261 : 1; /* [5] */
        unsigned int fifo_wr_int262 : 1; /* [6] */
        unsigned int fifo_wr_int263 : 1; /* [7] */
        unsigned int fifo_wr_int264 : 1; /* [8] */
        unsigned int fifo_wr_int265 : 1; /* [9] */
        unsigned int fifo_wr_int266 : 1; /* [10] */
        unsigned int fifo_wr_int267 : 1; /* [11] */
        unsigned int fifo_wr_int268 : 1; /* [12] */
        unsigned int fifo_wr_int269 : 1; /* [13] */
        unsigned int fifo_wr_int270 : 1; /* [14] */
        unsigned int fifo_wr_int271 : 1; /* [15] */
        unsigned int fifo_wr_int272 : 1; /* [16] */
        unsigned int fifo_wr_int273 : 1; /* [17] */
        unsigned int fifo_wr_int274 : 1; /* [18] */
        unsigned int fifo_wr_int275 : 1; /* [19] */
        unsigned int fifo_wr_int276 : 1; /* [20] */
        unsigned int fifo_wr_int277 : 1; /* [21] */
        unsigned int fifo_wr_int278 : 1; /* [22] */
        unsigned int fifo_wr_int279 : 1; /* [23] */
        unsigned int fifo_wr_int280 : 1; /* [24] */
        unsigned int fifo_wr_int281 : 1; /* [25] */
        unsigned int fifo_wr_int282 : 1; /* [26] */
        unsigned int fifo_wr_int283 : 1; /* [27] */
        unsigned int fifo_wr_int284 : 1; /* [28] */
        unsigned int fifo_wr_int285 : 1; /* [29] */
        unsigned int fifo_wr_int286 : 1; /* [30] */
        unsigned int fifo_wr_int287 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_wr_int8;

typedef union {
    struct {
        unsigned int fifo_wr_int288 : 1; /* [0] */
        unsigned int fifo_wr_int289 : 1; /* [1] */
        unsigned int fifo_wr_int290 : 1; /* [2] */
        unsigned int fifo_wr_int291 : 1; /* [3] */
        unsigned int fifo_wr_int292 : 1; /* [4] */
        unsigned int fifo_wr_int293 : 1; /* [5] */
        unsigned int fifo_wr_int294 : 1; /* [6] */
        unsigned int fifo_wr_int295 : 1; /* [7] */
        unsigned int fifo_wr_int296 : 1; /* [8] */
        unsigned int fifo_wr_int297 : 1; /* [9] */
        unsigned int fifo_wr_int298 : 1; /* [10] */
        unsigned int fifo_wr_int299 : 1; /* [11] */
        unsigned int fifo_wr_int300 : 1; /* [12] */
        unsigned int fifo_wr_int301 : 1; /* [13] */
        unsigned int fifo_wr_int302 : 1; /* [14] */
        unsigned int fifo_wr_int303 : 1; /* [15] */
        unsigned int fifo_wr_int304 : 1; /* [16] */
        unsigned int fifo_wr_int305 : 1; /* [17] */
        unsigned int fifo_wr_int306 : 1; /* [18] */
        unsigned int fifo_wr_int307 : 1; /* [19] */
        unsigned int fifo_wr_int308 : 1; /* [20] */
        unsigned int fifo_wr_int309 : 1; /* [21] */
        unsigned int fifo_wr_int310 : 1; /* [22] */
        unsigned int fifo_wr_int311 : 1; /* [23] */
        unsigned int fifo_wr_int312 : 1; /* [24] */
        unsigned int fifo_wr_int313 : 1; /* [25] */
        unsigned int fifo_wr_int314 : 1; /* [26] */
        unsigned int fifo_wr_int315 : 1; /* [27] */
        unsigned int fifo_wr_int316 : 1; /* [28] */
        unsigned int fifo_wr_int317 : 1; /* [29] */
        unsigned int fifo_wr_int318 : 1; /* [30] */
        unsigned int fifo_wr_int319 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_wr_int9;

typedef union {
    struct {
        unsigned int fifo_wr_int320 : 1; /* [0] */
        unsigned int fifo_wr_int321 : 1; /* [1] */
        unsigned int fifo_wr_int322 : 1; /* [2] */
        unsigned int fifo_wr_int323 : 1; /* [3] */
        unsigned int fifo_wr_int324 : 1; /* [4] */
        unsigned int fifo_wr_int325 : 1; /* [5] */
        unsigned int fifo_wr_int326 : 1; /* [6] */
        unsigned int fifo_wr_int327 : 1; /* [7] */
        unsigned int fifo_wr_int328 : 1; /* [8] */
        unsigned int fifo_wr_int329 : 1; /* [9] */
        unsigned int fifo_wr_int330 : 1; /* [10] */
        unsigned int fifo_wr_int331 : 1; /* [11] */
        unsigned int fifo_wr_int332 : 1; /* [12] */
        unsigned int fifo_wr_int333 : 1; /* [13] */
        unsigned int fifo_wr_int334 : 1; /* [14] */
        unsigned int fifo_wr_int335 : 1; /* [15] */
        unsigned int fifo_wr_int336 : 1; /* [16] */
        unsigned int fifo_wr_int337 : 1; /* [17] */
        unsigned int fifo_wr_int338 : 1; /* [18] */
        unsigned int fifo_wr_int339 : 1; /* [19] */
        unsigned int fifo_wr_int340 : 1; /* [20] */
        unsigned int fifo_wr_int341 : 1; /* [21] */
        unsigned int fifo_wr_int342 : 1; /* [22] */
        unsigned int fifo_wr_int343 : 1; /* [23] */
        unsigned int fifo_wr_int344 : 1; /* [24] */
        unsigned int fifo_wr_int345 : 1; /* [25] */
        unsigned int fifo_wr_int346 : 1; /* [26] */
        unsigned int fifo_wr_int347 : 1; /* [27] */
        unsigned int fifo_wr_int348 : 1; /* [28] */
        unsigned int fifo_wr_int349 : 1; /* [29] */
        unsigned int fifo_wr_int350 : 1; /* [30] */
        unsigned int fifo_wr_int351 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_wr_int10;

typedef union {
    struct {
        unsigned int fifo_wr_int352 : 1; /* [0] */
        unsigned int fifo_wr_int353 : 1; /* [1] */
        unsigned int fifo_wr_int354 : 1; /* [2] */
        unsigned int fifo_wr_int355 : 1; /* [3] */
        unsigned int fifo_wr_int356 : 1; /* [4] */
        unsigned int fifo_wr_int357 : 1; /* [5] */
        unsigned int fifo_wr_int358 : 1; /* [6] */
        unsigned int fifo_wr_int359 : 1; /* [7] */
        unsigned int fifo_wr_int360 : 1; /* [8] */
        unsigned int fifo_wr_int361 : 1; /* [9] */
        unsigned int fifo_wr_int362 : 1; /* [10] */
        unsigned int fifo_wr_int363 : 1; /* [11] */
        unsigned int fifo_wr_int364 : 1; /* [12] */
        unsigned int fifo_wr_int365 : 1; /* [13] */
        unsigned int fifo_wr_int366 : 1; /* [14] */
        unsigned int fifo_wr_int367 : 1; /* [15] */
        unsigned int fifo_wr_int368 : 1; /* [16] */
        unsigned int fifo_wr_int369 : 1; /* [17] */
        unsigned int fifo_wr_int370 : 1; /* [18] */
        unsigned int fifo_wr_int371 : 1; /* [19] */
        unsigned int fifo_wr_int372 : 1; /* [20] */
        unsigned int fifo_wr_int373 : 1; /* [21] */
        unsigned int fifo_wr_int374 : 1; /* [22] */
        unsigned int fifo_wr_int375 : 1; /* [23] */
        unsigned int fifo_wr_int376 : 1; /* [24] */
        unsigned int fifo_wr_int377 : 1; /* [25] */
        unsigned int fifo_wr_int378 : 1; /* [26] */
        unsigned int fifo_wr_int379 : 1; /* [27] */
        unsigned int fifo_wr_int380 : 1; /* [28] */
        unsigned int fifo_wr_int381 : 1; /* [29] */
        unsigned int fifo_wr_int382 : 1; /* [30] */
        unsigned int fifo_wr_int383 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_wr_int11;

typedef union {
    struct {
        unsigned int fifo_wr_int384 : 1; /* [0] */
        unsigned int fifo_wr_int385 : 1; /* [1] */
        unsigned int fifo_wr_int386 : 1; /* [2] */
        unsigned int fifo_wr_int387 : 1; /* [3] */
        unsigned int fifo_wr_int388 : 1; /* [4] */
        unsigned int fifo_wr_int389 : 1; /* [5] */
        unsigned int fifo_wr_int390 : 1; /* [6] */
        unsigned int fifo_wr_int391 : 1; /* [7] */
        unsigned int fifo_wr_int392 : 1; /* [8] */
        unsigned int fifo_wr_int393 : 1; /* [9] */
        unsigned int fifo_wr_int394 : 1; /* [10] */
        unsigned int fifo_wr_int395 : 1; /* [11] */
        unsigned int fifo_wr_int396 : 1; /* [12] */
        unsigned int fifo_wr_int397 : 1; /* [13] */
        unsigned int fifo_wr_int398 : 1; /* [14] */
        unsigned int fifo_wr_int399 : 1; /* [15] */
        unsigned int fifo_wr_int400 : 1; /* [16] */
        unsigned int fifo_wr_int401 : 1; /* [17] */
        unsigned int fifo_wr_int402 : 1; /* [18] */
        unsigned int fifo_wr_int403 : 1; /* [19] */
        unsigned int fifo_wr_int404 : 1; /* [20] */
        unsigned int fifo_wr_int405 : 1; /* [21] */
        unsigned int fifo_wr_int406 : 1; /* [22] */
        unsigned int fifo_wr_int407 : 1; /* [23] */
        unsigned int fifo_wr_int408 : 1; /* [24] */
        unsigned int fifo_wr_int409 : 1; /* [25] */
        unsigned int fifo_wr_int410 : 1; /* [26] */
        unsigned int fifo_wr_int411 : 1; /* [27] */
        unsigned int fifo_wr_int412 : 1; /* [28] */
        unsigned int fifo_wr_int413 : 1; /* [29] */
        unsigned int fifo_wr_int414 : 1; /* [30] */
        unsigned int fifo_wr_int415 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_wr_int12;

typedef union {
    struct {
        unsigned int fifo_wr_int416 : 1; /* [0] */
        unsigned int fifo_wr_int417 : 1; /* [1] */
        unsigned int fifo_wr_int418 : 1; /* [2] */
        unsigned int fifo_wr_int419 : 1; /* [3] */
        unsigned int fifo_wr_int420 : 1; /* [4] */
        unsigned int fifo_wr_int421 : 1; /* [5] */
        unsigned int fifo_wr_int422 : 1; /* [6] */
        unsigned int fifo_wr_int423 : 1; /* [7] */
        unsigned int fifo_wr_int424 : 1; /* [8] */
        unsigned int fifo_wr_int425 : 1; /* [9] */
        unsigned int fifo_wr_int426 : 1; /* [10] */
        unsigned int fifo_wr_int427 : 1; /* [11] */
        unsigned int fifo_wr_int428 : 1; /* [12] */
        unsigned int fifo_wr_int429 : 1; /* [13] */
        unsigned int fifo_wr_int430 : 1; /* [14] */
        unsigned int fifo_wr_int431 : 1; /* [15] */
        unsigned int fifo_wr_int432 : 1; /* [16] */
        unsigned int fifo_wr_int433 : 1; /* [17] */
        unsigned int fifo_wr_int434 : 1; /* [18] */
        unsigned int fifo_wr_int435 : 1; /* [19] */
        unsigned int fifo_wr_int436 : 1; /* [20] */
        unsigned int fifo_wr_int437 : 1; /* [21] */
        unsigned int fifo_wr_int438 : 1; /* [22] */
        unsigned int fifo_wr_int439 : 1; /* [23] */
        unsigned int fifo_wr_int440 : 1; /* [24] */
        unsigned int fifo_wr_int441 : 1; /* [25] */
        unsigned int fifo_wr_int442 : 1; /* [26] */
        unsigned int fifo_wr_int443 : 1; /* [27] */
        unsigned int fifo_wr_int444 : 1; /* [28] */
        unsigned int fifo_wr_int445 : 1; /* [29] */
        unsigned int fifo_wr_int446 : 1; /* [30] */
        unsigned int fifo_wr_int447 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_wr_int13;

typedef union {
    struct {
        unsigned int fifo_wr_int448 : 1; /* [0] */
        unsigned int fifo_wr_int449 : 1; /* [1] */
        unsigned int fifo_wr_int450 : 1; /* [2] */
        unsigned int fifo_wr_int451 : 1; /* [3] */
        unsigned int fifo_wr_int452 : 1; /* [4] */
        unsigned int fifo_wr_int453 : 1; /* [5] */
        unsigned int fifo_wr_int454 : 1; /* [6] */
        unsigned int fifo_wr_int455 : 1; /* [7] */
        unsigned int fifo_wr_int456 : 1; /* [8] */
        unsigned int fifo_wr_int457 : 1; /* [9] */
        unsigned int fifo_wr_int458 : 1; /* [10] */
        unsigned int fifo_wr_int459 : 1; /* [11] */
        unsigned int fifo_wr_int460 : 1; /* [12] */
        unsigned int fifo_wr_int461 : 1; /* [13] */
        unsigned int fifo_wr_int462 : 1; /* [14] */
        unsigned int fifo_wr_int463 : 1; /* [15] */
        unsigned int fifo_wr_int464 : 1; /* [16] */
        unsigned int fifo_wr_int465 : 1; /* [17] */
        unsigned int fifo_wr_int466 : 1; /* [18] */
        unsigned int fifo_wr_int467 : 1; /* [19] */
        unsigned int fifo_wr_int468 : 1; /* [20] */
        unsigned int fifo_wr_int469 : 1; /* [21] */
        unsigned int fifo_wr_int470 : 1; /* [22] */
        unsigned int fifo_wr_int471 : 1; /* [23] */
        unsigned int fifo_wr_int472 : 1; /* [24] */
        unsigned int fifo_wr_int473 : 1; /* [25] */
        unsigned int fifo_wr_int474 : 1; /* [26] */
        unsigned int fifo_wr_int475 : 1; /* [27] */
        unsigned int fifo_wr_int476 : 1; /* [28] */
        unsigned int fifo_wr_int477 : 1; /* [29] */
        unsigned int fifo_wr_int478 : 1; /* [30] */
        unsigned int fifo_wr_int479 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_wr_int14;

typedef union {
    struct {
        unsigned int fifo_wr_int480 : 1; /* [0] */
        unsigned int fifo_wr_int481 : 1; /* [1] */
        unsigned int fifo_wr_int482 : 1; /* [2] */
        unsigned int fifo_wr_int483 : 1; /* [3] */
        unsigned int fifo_wr_int484 : 1; /* [4] */
        unsigned int fifo_wr_int485 : 1; /* [5] */
        unsigned int fifo_wr_int486 : 1; /* [6] */
        unsigned int fifo_wr_int487 : 1; /* [7] */
        unsigned int fifo_wr_int488 : 1; /* [8] */
        unsigned int fifo_wr_int489 : 1; /* [9] */
        unsigned int fifo_wr_int490 : 1; /* [10] */
        unsigned int fifo_wr_int491 : 1; /* [11] */
        unsigned int fifo_wr_int492 : 1; /* [12] */
        unsigned int fifo_wr_int493 : 1; /* [13] */
        unsigned int fifo_wr_int494 : 1; /* [14] */
        unsigned int fifo_wr_int495 : 1; /* [15] */
        unsigned int fifo_wr_int496 : 1; /* [16] */
        unsigned int fifo_wr_int497 : 1; /* [17] */
        unsigned int fifo_wr_int498 : 1; /* [18] */
        unsigned int fifo_wr_int499 : 1; /* [19] */
        unsigned int fifo_wr_int500 : 1; /* [20] */
        unsigned int fifo_wr_int501 : 1; /* [21] */
        unsigned int fifo_wr_int502 : 1; /* [22] */
        unsigned int fifo_wr_int503 : 1; /* [23] */
        unsigned int fifo_wr_int504 : 1; /* [24] */
        unsigned int fifo_wr_int505 : 1; /* [25] */
        unsigned int fifo_wr_int506 : 1; /* [26] */
        unsigned int fifo_wr_int507 : 1; /* [27] */
        unsigned int fifo_wr_int508 : 1; /* [28] */
        unsigned int fifo_wr_int509 : 1; /* [29] */
        unsigned int fifo_wr_int510 : 1; /* [30] */
        unsigned int fifo_wr_int511 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_wr_int15;

typedef union {
    struct {
        unsigned int fifo_wr_int512 : 1; /* [0] */
        unsigned int fifo_wr_int513 : 1; /* [1] */
        unsigned int fifo_wr_int514 : 1; /* [2] */
        unsigned int fifo_wr_int515 : 1; /* [3] */
        unsigned int fifo_wr_int516 : 1; /* [4] */
        unsigned int fifo_wr_int517 : 1; /* [5] */
        unsigned int fifo_wr_int518 : 1; /* [6] */
        unsigned int fifo_wr_int519 : 1; /* [7] */
        unsigned int fifo_wr_int520 : 1; /* [8] */
        unsigned int fifo_wr_int521 : 1; /* [9] */
        unsigned int fifo_wr_int522 : 1; /* [10] */
        unsigned int fifo_wr_int523 : 1; /* [11] */
        unsigned int fifo_wr_int524 : 1; /* [12] */
        unsigned int fifo_wr_int525 : 1; /* [13] */
        unsigned int fifo_wr_int526 : 1; /* [14] */
        unsigned int fifo_wr_int527 : 1; /* [15] */
        unsigned int fifo_wr_int528 : 1; /* [16] */
        unsigned int fifo_wr_int529 : 1; /* [17] */
        unsigned int fifo_wr_int530 : 1; /* [18] */
        unsigned int fifo_wr_int531 : 1; /* [19] */
        unsigned int fifo_wr_int532 : 1; /* [20] */
        unsigned int fifo_wr_int533 : 1; /* [21] */
        unsigned int fifo_wr_int534 : 1; /* [22] */
        unsigned int fifo_wr_int535 : 1; /* [23] */
        unsigned int fifo_wr_int536 : 1; /* [24] */
        unsigned int fifo_wr_int537 : 1; /* [25] */
        unsigned int fifo_wr_int538 : 1; /* [26] */
        unsigned int fifo_wr_int539 : 1; /* [27] */
        unsigned int fifo_wr_int540 : 1; /* [28] */
        unsigned int fifo_wr_int541 : 1; /* [29] */
        unsigned int fifo_wr_int542 : 1; /* [30] */
        unsigned int fifo_wr_int543 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_wr_int16;

typedef union {
    struct {
        unsigned int fifo_wr_int544 : 1; /* [0] */
        unsigned int fifo_wr_int545 : 1; /* [1] */
        unsigned int fifo_wr_int546 : 1; /* [2] */
        unsigned int fifo_wr_int547 : 1; /* [3] */
        unsigned int fifo_wr_int548 : 1; /* [4] */
        unsigned int fifo_wr_int549 : 1; /* [5] */
        unsigned int fifo_wr_int550 : 1; /* [6] */
        unsigned int fifo_wr_int551 : 1; /* [7] */
        unsigned int fifo_wr_int552 : 1; /* [8] */
        unsigned int fifo_wr_int553 : 1; /* [9] */
        unsigned int fifo_wr_int554 : 1; /* [10] */
        unsigned int fifo_wr_int555 : 1; /* [11] */
        unsigned int fifo_wr_int556 : 1; /* [12] */
        unsigned int fifo_wr_int557 : 1; /* [13] */
        unsigned int fifo_wr_int558 : 1; /* [14] */
        unsigned int fifo_wr_int559 : 1; /* [15] */
        unsigned int fifo_wr_int560 : 1; /* [16] */
        unsigned int fifo_wr_int561 : 1; /* [17] */
        unsigned int fifo_wr_int562 : 1; /* [18] */
        unsigned int fifo_wr_int563 : 1; /* [19] */
        unsigned int fifo_wr_int564 : 1; /* [20] */
        unsigned int fifo_wr_int565 : 1; /* [21] */
        unsigned int fifo_wr_int566 : 1; /* [22] */
        unsigned int fifo_wr_int567 : 1; /* [23] */
        unsigned int fifo_wr_int568 : 1; /* [24] */
        unsigned int fifo_wr_int569 : 1; /* [25] */
        unsigned int fifo_wr_int570 : 1; /* [26] */
        unsigned int fifo_wr_int571 : 1; /* [27] */
        unsigned int fifo_wr_int572 : 1; /* [28] */
        unsigned int fifo_wr_int573 : 1; /* [29] */
        unsigned int fifo_wr_int574 : 1; /* [30] */
        unsigned int fifo_wr_int575 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_wr_int17;

typedef union {
    struct {
        unsigned int fifo_wr_int576 : 1; /* [0] */
        unsigned int fifo_wr_int577 : 1; /* [1] */
        unsigned int fifo_wr_int578 : 1; /* [2] */
        unsigned int fifo_wr_int579 : 1; /* [3] */
        unsigned int fifo_wr_int580 : 1; /* [4] */
        unsigned int fifo_wr_int581 : 1; /* [5] */
        unsigned int fifo_wr_int582 : 1; /* [6] */
        unsigned int fifo_wr_int583 : 1; /* [7] */
        unsigned int fifo_wr_int584 : 1; /* [8] */
        unsigned int fifo_wr_int585 : 1; /* [9] */
        unsigned int fifo_wr_int586 : 1; /* [10] */
        unsigned int fifo_wr_int587 : 1; /* [11] */
        unsigned int fifo_wr_int588 : 1; /* [12] */
        unsigned int fifo_wr_int589 : 1; /* [13] */
        unsigned int fifo_wr_int590 : 1; /* [14] */
        unsigned int fifo_wr_int591 : 1; /* [15] */
        unsigned int fifo_wr_int592 : 1; /* [16] */
        unsigned int fifo_wr_int593 : 1; /* [17] */
        unsigned int fifo_wr_int594 : 1; /* [18] */
        unsigned int fifo_wr_int595 : 1; /* [19] */
        unsigned int fifo_wr_int596 : 1; /* [20] */
        unsigned int fifo_wr_int597 : 1; /* [21] */
        unsigned int fifo_wr_int598 : 1; /* [22] */
        unsigned int fifo_wr_int599 : 1; /* [23] */
        unsigned int fifo_wr_int600 : 1; /* [24] */
        unsigned int fifo_wr_int601 : 1; /* [25] */
        unsigned int fifo_wr_int602 : 1; /* [26] */
        unsigned int fifo_wr_int603 : 1; /* [27] */
        unsigned int fifo_wr_int604 : 1; /* [28] */
        unsigned int fifo_wr_int605 : 1; /* [29] */
        unsigned int fifo_wr_int606 : 1; /* [30] */
        unsigned int fifo_wr_int607 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_wr_int18;

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
} u_tpu_fifo_rd_int0;

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
} u_tpu_fifo_rd_int1;

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
} u_tpu_fifo_rd_int2;

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
} u_tpu_fifo_rd_int3;

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
} u_tpu_fifo_rd_int4;

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
} u_tpu_fifo_rd_int5;

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
} u_tpu_fifo_rd_int6;

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
} u_tpu_fifo_rd_int7;

typedef union {
    struct {
        unsigned int fifo_rd_int256 : 1; /* [0] */
        unsigned int fifo_rd_int257 : 1; /* [1] */
        unsigned int fifo_rd_int258 : 1; /* [2] */
        unsigned int fifo_rd_int259 : 1; /* [3] */
        unsigned int fifo_rd_int260 : 1; /* [4] */
        unsigned int fifo_rd_int261 : 1; /* [5] */
        unsigned int fifo_rd_int262 : 1; /* [6] */
        unsigned int fifo_rd_int263 : 1; /* [7] */
        unsigned int fifo_rd_int264 : 1; /* [8] */
        unsigned int fifo_rd_int265 : 1; /* [9] */
        unsigned int fifo_rd_int266 : 1; /* [10] */
        unsigned int fifo_rd_int267 : 1; /* [11] */
        unsigned int fifo_rd_int268 : 1; /* [12] */
        unsigned int fifo_rd_int269 : 1; /* [13] */
        unsigned int fifo_rd_int270 : 1; /* [14] */
        unsigned int fifo_rd_int271 : 1; /* [15] */
        unsigned int fifo_rd_int272 : 1; /* [16] */
        unsigned int fifo_rd_int273 : 1; /* [17] */
        unsigned int fifo_rd_int274 : 1; /* [18] */
        unsigned int fifo_rd_int275 : 1; /* [19] */
        unsigned int fifo_rd_int276 : 1; /* [20] */
        unsigned int fifo_rd_int277 : 1; /* [21] */
        unsigned int fifo_rd_int278 : 1; /* [22] */
        unsigned int fifo_rd_int279 : 1; /* [23] */
        unsigned int fifo_rd_int280 : 1; /* [24] */
        unsigned int fifo_rd_int281 : 1; /* [25] */
        unsigned int fifo_rd_int282 : 1; /* [26] */
        unsigned int fifo_rd_int283 : 1; /* [27] */
        unsigned int fifo_rd_int284 : 1; /* [28] */
        unsigned int fifo_rd_int285 : 1; /* [29] */
        unsigned int fifo_rd_int286 : 1; /* [30] */
        unsigned int fifo_rd_int287 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_rd_int8;

typedef union {
    struct {
        unsigned int fifo_rd_int288 : 1; /* [0] */
        unsigned int fifo_rd_int289 : 1; /* [1] */
        unsigned int fifo_rd_int290 : 1; /* [2] */
        unsigned int fifo_rd_int291 : 1; /* [3] */
        unsigned int fifo_rd_int292 : 1; /* [4] */
        unsigned int fifo_rd_int293 : 1; /* [5] */
        unsigned int fifo_rd_int294 : 1; /* [6] */
        unsigned int fifo_rd_int295 : 1; /* [7] */
        unsigned int fifo_rd_int296 : 1; /* [8] */
        unsigned int fifo_rd_int297 : 1; /* [9] */
        unsigned int fifo_rd_int298 : 1; /* [10] */
        unsigned int fifo_rd_int299 : 1; /* [11] */
        unsigned int fifo_rd_int300 : 1; /* [12] */
        unsigned int fifo_rd_int301 : 1; /* [13] */
        unsigned int fifo_rd_int302 : 1; /* [14] */
        unsigned int fifo_rd_int303 : 1; /* [15] */
        unsigned int fifo_rd_int304 : 1; /* [16] */
        unsigned int fifo_rd_int305 : 1; /* [17] */
        unsigned int fifo_rd_int306 : 1; /* [18] */
        unsigned int fifo_rd_int307 : 1; /* [19] */
        unsigned int fifo_rd_int308 : 1; /* [20] */
        unsigned int fifo_rd_int309 : 1; /* [21] */
        unsigned int fifo_rd_int310 : 1; /* [22] */
        unsigned int fifo_rd_int311 : 1; /* [23] */
        unsigned int fifo_rd_int312 : 1; /* [24] */
        unsigned int fifo_rd_int313 : 1; /* [25] */
        unsigned int fifo_rd_int314 : 1; /* [26] */
        unsigned int fifo_rd_int315 : 1; /* [27] */
        unsigned int fifo_rd_int316 : 1; /* [28] */
        unsigned int fifo_rd_int317 : 1; /* [29] */
        unsigned int fifo_rd_int318 : 1; /* [30] */
        unsigned int fifo_rd_int319 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_rd_int9;

typedef union {
    struct {
        unsigned int fifo_rd_int320 : 1; /* [0] */
        unsigned int fifo_rd_int321 : 1; /* [1] */
        unsigned int fifo_rd_int322 : 1; /* [2] */
        unsigned int fifo_rd_int323 : 1; /* [3] */
        unsigned int fifo_rd_int324 : 1; /* [4] */
        unsigned int fifo_rd_int325 : 1; /* [5] */
        unsigned int fifo_rd_int326 : 1; /* [6] */
        unsigned int fifo_rd_int327 : 1; /* [7] */
        unsigned int fifo_rd_int328 : 1; /* [8] */
        unsigned int fifo_rd_int329 : 1; /* [9] */
        unsigned int fifo_rd_int330 : 1; /* [10] */
        unsigned int fifo_rd_int331 : 1; /* [11] */
        unsigned int fifo_rd_int332 : 1; /* [12] */
        unsigned int fifo_rd_int333 : 1; /* [13] */
        unsigned int fifo_rd_int334 : 1; /* [14] */
        unsigned int fifo_rd_int335 : 1; /* [15] */
        unsigned int fifo_rd_int336 : 1; /* [16] */
        unsigned int fifo_rd_int337 : 1; /* [17] */
        unsigned int fifo_rd_int338 : 1; /* [18] */
        unsigned int fifo_rd_int339 : 1; /* [19] */
        unsigned int fifo_rd_int340 : 1; /* [20] */
        unsigned int fifo_rd_int341 : 1; /* [21] */
        unsigned int fifo_rd_int342 : 1; /* [22] */
        unsigned int fifo_rd_int343 : 1; /* [23] */
        unsigned int fifo_rd_int344 : 1; /* [24] */
        unsigned int fifo_rd_int345 : 1; /* [25] */
        unsigned int fifo_rd_int346 : 1; /* [26] */
        unsigned int fifo_rd_int347 : 1; /* [27] */
        unsigned int fifo_rd_int348 : 1; /* [28] */
        unsigned int fifo_rd_int349 : 1; /* [29] */
        unsigned int fifo_rd_int350 : 1; /* [30] */
        unsigned int fifo_rd_int351 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_rd_int10;

typedef union {
    struct {
        unsigned int fifo_rd_int352 : 1; /* [0] */
        unsigned int fifo_rd_int353 : 1; /* [1] */
        unsigned int fifo_rd_int354 : 1; /* [2] */
        unsigned int fifo_rd_int355 : 1; /* [3] */
        unsigned int fifo_rd_int356 : 1; /* [4] */
        unsigned int fifo_rd_int357 : 1; /* [5] */
        unsigned int fifo_rd_int358 : 1; /* [6] */
        unsigned int fifo_rd_int359 : 1; /* [7] */
        unsigned int fifo_rd_int360 : 1; /* [8] */
        unsigned int fifo_rd_int361 : 1; /* [9] */
        unsigned int fifo_rd_int362 : 1; /* [10] */
        unsigned int fifo_rd_int363 : 1; /* [11] */
        unsigned int fifo_rd_int364 : 1; /* [12] */
        unsigned int fifo_rd_int365 : 1; /* [13] */
        unsigned int fifo_rd_int366 : 1; /* [14] */
        unsigned int fifo_rd_int367 : 1; /* [15] */
        unsigned int fifo_rd_int368 : 1; /* [16] */
        unsigned int fifo_rd_int369 : 1; /* [17] */
        unsigned int fifo_rd_int370 : 1; /* [18] */
        unsigned int fifo_rd_int371 : 1; /* [19] */
        unsigned int fifo_rd_int372 : 1; /* [20] */
        unsigned int fifo_rd_int373 : 1; /* [21] */
        unsigned int fifo_rd_int374 : 1; /* [22] */
        unsigned int fifo_rd_int375 : 1; /* [23] */
        unsigned int fifo_rd_int376 : 1; /* [24] */
        unsigned int fifo_rd_int377 : 1; /* [25] */
        unsigned int fifo_rd_int378 : 1; /* [26] */
        unsigned int fifo_rd_int379 : 1; /* [27] */
        unsigned int fifo_rd_int380 : 1; /* [28] */
        unsigned int fifo_rd_int381 : 1; /* [29] */
        unsigned int fifo_rd_int382 : 1; /* [30] */
        unsigned int fifo_rd_int383 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_rd_int11;

typedef union {
    struct {
        unsigned int fifo_rd_int384 : 1; /* [0] */
        unsigned int fifo_rd_int385 : 1; /* [1] */
        unsigned int fifo_rd_int386 : 1; /* [2] */
        unsigned int fifo_rd_int387 : 1; /* [3] */
        unsigned int fifo_rd_int388 : 1; /* [4] */
        unsigned int fifo_rd_int389 : 1; /* [5] */
        unsigned int fifo_rd_int390 : 1; /* [6] */
        unsigned int fifo_rd_int391 : 1; /* [7] */
        unsigned int fifo_rd_int392 : 1; /* [8] */
        unsigned int fifo_rd_int393 : 1; /* [9] */
        unsigned int fifo_rd_int394 : 1; /* [10] */
        unsigned int fifo_rd_int395 : 1; /* [11] */
        unsigned int fifo_rd_int396 : 1; /* [12] */
        unsigned int fifo_rd_int397 : 1; /* [13] */
        unsigned int fifo_rd_int398 : 1; /* [14] */
        unsigned int fifo_rd_int399 : 1; /* [15] */
        unsigned int fifo_rd_int400 : 1; /* [16] */
        unsigned int fifo_rd_int401 : 1; /* [17] */
        unsigned int fifo_rd_int402 : 1; /* [18] */
        unsigned int fifo_rd_int403 : 1; /* [19] */
        unsigned int fifo_rd_int404 : 1; /* [20] */
        unsigned int fifo_rd_int405 : 1; /* [21] */
        unsigned int fifo_rd_int406 : 1; /* [22] */
        unsigned int fifo_rd_int407 : 1; /* [23] */
        unsigned int fifo_rd_int408 : 1; /* [24] */
        unsigned int fifo_rd_int409 : 1; /* [25] */
        unsigned int fifo_rd_int410 : 1; /* [26] */
        unsigned int fifo_rd_int411 : 1; /* [27] */
        unsigned int fifo_rd_int412 : 1; /* [28] */
        unsigned int fifo_rd_int413 : 1; /* [29] */
        unsigned int fifo_rd_int414 : 1; /* [30] */
        unsigned int fifo_rd_int415 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_rd_int12;

typedef union {
    struct {
        unsigned int fifo_rd_int416 : 1; /* [0] */
        unsigned int fifo_rd_int417 : 1; /* [1] */
        unsigned int fifo_rd_int418 : 1; /* [2] */
        unsigned int fifo_rd_int419 : 1; /* [3] */
        unsigned int fifo_rd_int420 : 1; /* [4] */
        unsigned int fifo_rd_int421 : 1; /* [5] */
        unsigned int fifo_rd_int422 : 1; /* [6] */
        unsigned int fifo_rd_int423 : 1; /* [7] */
        unsigned int fifo_rd_int424 : 1; /* [8] */
        unsigned int fifo_rd_int425 : 1; /* [9] */
        unsigned int fifo_rd_int426 : 1; /* [10] */
        unsigned int fifo_rd_int427 : 1; /* [11] */
        unsigned int fifo_rd_int428 : 1; /* [12] */
        unsigned int fifo_rd_int429 : 1; /* [13] */
        unsigned int fifo_rd_int430 : 1; /* [14] */
        unsigned int fifo_rd_int431 : 1; /* [15] */
        unsigned int fifo_rd_int432 : 1; /* [16] */
        unsigned int fifo_rd_int433 : 1; /* [17] */
        unsigned int fifo_rd_int434 : 1; /* [18] */
        unsigned int fifo_rd_int435 : 1; /* [19] */
        unsigned int fifo_rd_int436 : 1; /* [20] */
        unsigned int fifo_rd_int437 : 1; /* [21] */
        unsigned int fifo_rd_int438 : 1; /* [22] */
        unsigned int fifo_rd_int439 : 1; /* [23] */
        unsigned int fifo_rd_int440 : 1; /* [24] */
        unsigned int fifo_rd_int441 : 1; /* [25] */
        unsigned int fifo_rd_int442 : 1; /* [26] */
        unsigned int fifo_rd_int443 : 1; /* [27] */
        unsigned int fifo_rd_int444 : 1; /* [28] */
        unsigned int fifo_rd_int445 : 1; /* [29] */
        unsigned int fifo_rd_int446 : 1; /* [30] */
        unsigned int fifo_rd_int447 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_rd_int13;

typedef union {
    struct {
        unsigned int fifo_rd_int448 : 1; /* [0] */
        unsigned int fifo_rd_int449 : 1; /* [1] */
        unsigned int fifo_rd_int450 : 1; /* [2] */
        unsigned int fifo_rd_int451 : 1; /* [3] */
        unsigned int fifo_rd_int452 : 1; /* [4] */
        unsigned int fifo_rd_int453 : 1; /* [5] */
        unsigned int fifo_rd_int454 : 1; /* [6] */
        unsigned int fifo_rd_int455 : 1; /* [7] */
        unsigned int fifo_rd_int456 : 1; /* [8] */
        unsigned int fifo_rd_int457 : 1; /* [9] */
        unsigned int fifo_rd_int458 : 1; /* [10] */
        unsigned int fifo_rd_int459 : 1; /* [11] */
        unsigned int fifo_rd_int460 : 1; /* [12] */
        unsigned int fifo_rd_int461 : 1; /* [13] */
        unsigned int fifo_rd_int462 : 1; /* [14] */
        unsigned int fifo_rd_int463 : 1; /* [15] */
        unsigned int fifo_rd_int464 : 1; /* [16] */
        unsigned int fifo_rd_int465 : 1; /* [17] */
        unsigned int fifo_rd_int466 : 1; /* [18] */
        unsigned int fifo_rd_int467 : 1; /* [19] */
        unsigned int fifo_rd_int468 : 1; /* [20] */
        unsigned int fifo_rd_int469 : 1; /* [21] */
        unsigned int fifo_rd_int470 : 1; /* [22] */
        unsigned int fifo_rd_int471 : 1; /* [23] */
        unsigned int fifo_rd_int472 : 1; /* [24] */
        unsigned int fifo_rd_int473 : 1; /* [25] */
        unsigned int fifo_rd_int474 : 1; /* [26] */
        unsigned int fifo_rd_int475 : 1; /* [27] */
        unsigned int fifo_rd_int476 : 1; /* [28] */
        unsigned int fifo_rd_int477 : 1; /* [29] */
        unsigned int fifo_rd_int478 : 1; /* [30] */
        unsigned int fifo_rd_int479 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_rd_int14;

typedef union {
    struct {
        unsigned int fifo_rd_int480 : 1; /* [0] */
        unsigned int fifo_rd_int481 : 1; /* [1] */
        unsigned int fifo_rd_int482 : 1; /* [2] */
        unsigned int fifo_rd_int483 : 1; /* [3] */
        unsigned int fifo_rd_int484 : 1; /* [4] */
        unsigned int fifo_rd_int485 : 1; /* [5] */
        unsigned int fifo_rd_int486 : 1; /* [6] */
        unsigned int fifo_rd_int487 : 1; /* [7] */
        unsigned int fifo_rd_int488 : 1; /* [8] */
        unsigned int fifo_rd_int489 : 1; /* [9] */
        unsigned int fifo_rd_int490 : 1; /* [10] */
        unsigned int fifo_rd_int491 : 1; /* [11] */
        unsigned int fifo_rd_int492 : 1; /* [12] */
        unsigned int fifo_rd_int493 : 1; /* [13] */
        unsigned int fifo_rd_int494 : 1; /* [14] */
        unsigned int fifo_rd_int495 : 1; /* [15] */
        unsigned int fifo_rd_int496 : 1; /* [16] */
        unsigned int fifo_rd_int497 : 1; /* [17] */
        unsigned int fifo_rd_int498 : 1; /* [18] */
        unsigned int fifo_rd_int499 : 1; /* [19] */
        unsigned int fifo_rd_int500 : 1; /* [20] */
        unsigned int fifo_rd_int501 : 1; /* [21] */
        unsigned int fifo_rd_int502 : 1; /* [22] */
        unsigned int fifo_rd_int503 : 1; /* [23] */
        unsigned int fifo_rd_int504 : 1; /* [24] */
        unsigned int fifo_rd_int505 : 1; /* [25] */
        unsigned int fifo_rd_int506 : 1; /* [26] */
        unsigned int fifo_rd_int507 : 1; /* [27] */
        unsigned int fifo_rd_int508 : 1; /* [28] */
        unsigned int fifo_rd_int509 : 1; /* [29] */
        unsigned int fifo_rd_int510 : 1; /* [30] */
        unsigned int fifo_rd_int511 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_rd_int15;

typedef union {
    struct {
        unsigned int fifo_rd_int512 : 1; /* [0] */
        unsigned int fifo_rd_int513 : 1; /* [1] */
        unsigned int fifo_rd_int514 : 1; /* [2] */
        unsigned int fifo_rd_int515 : 1; /* [3] */
        unsigned int fifo_rd_int516 : 1; /* [4] */
        unsigned int fifo_rd_int517 : 1; /* [5] */
        unsigned int fifo_rd_int518 : 1; /* [6] */
        unsigned int fifo_rd_int519 : 1; /* [7] */
        unsigned int fifo_rd_int520 : 1; /* [8] */
        unsigned int fifo_rd_int521 : 1; /* [9] */
        unsigned int fifo_rd_int522 : 1; /* [10] */
        unsigned int fifo_rd_int523 : 1; /* [11] */
        unsigned int fifo_rd_int524 : 1; /* [12] */
        unsigned int fifo_rd_int525 : 1; /* [13] */
        unsigned int fifo_rd_int526 : 1; /* [14] */
        unsigned int fifo_rd_int527 : 1; /* [15] */
        unsigned int fifo_rd_int528 : 1; /* [16] */
        unsigned int fifo_rd_int529 : 1; /* [17] */
        unsigned int fifo_rd_int530 : 1; /* [18] */
        unsigned int fifo_rd_int531 : 1; /* [19] */
        unsigned int fifo_rd_int532 : 1; /* [20] */
        unsigned int fifo_rd_int533 : 1; /* [21] */
        unsigned int fifo_rd_int534 : 1; /* [22] */
        unsigned int fifo_rd_int535 : 1; /* [23] */
        unsigned int fifo_rd_int536 : 1; /* [24] */
        unsigned int fifo_rd_int537 : 1; /* [25] */
        unsigned int fifo_rd_int538 : 1; /* [26] */
        unsigned int fifo_rd_int539 : 1; /* [27] */
        unsigned int fifo_rd_int540 : 1; /* [28] */
        unsigned int fifo_rd_int541 : 1; /* [29] */
        unsigned int fifo_rd_int542 : 1; /* [30] */
        unsigned int fifo_rd_int543 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_rd_int16;

typedef union {
    struct {
        unsigned int fifo_rd_int544 : 1; /* [0] */
        unsigned int fifo_rd_int545 : 1; /* [1] */
        unsigned int fifo_rd_int546 : 1; /* [2] */
        unsigned int fifo_rd_int547 : 1; /* [3] */
        unsigned int fifo_rd_int548 : 1; /* [4] */
        unsigned int fifo_rd_int549 : 1; /* [5] */
        unsigned int fifo_rd_int550 : 1; /* [6] */
        unsigned int fifo_rd_int551 : 1; /* [7] */
        unsigned int fifo_rd_int552 : 1; /* [8] */
        unsigned int fifo_rd_int553 : 1; /* [9] */
        unsigned int fifo_rd_int554 : 1; /* [10] */
        unsigned int fifo_rd_int555 : 1; /* [11] */
        unsigned int fifo_rd_int556 : 1; /* [12] */
        unsigned int fifo_rd_int557 : 1; /* [13] */
        unsigned int fifo_rd_int558 : 1; /* [14] */
        unsigned int fifo_rd_int559 : 1; /* [15] */
        unsigned int fifo_rd_int560 : 1; /* [16] */
        unsigned int fifo_rd_int561 : 1; /* [17] */
        unsigned int fifo_rd_int562 : 1; /* [18] */
        unsigned int fifo_rd_int563 : 1; /* [19] */
        unsigned int fifo_rd_int564 : 1; /* [20] */
        unsigned int fifo_rd_int565 : 1; /* [21] */
        unsigned int fifo_rd_int566 : 1; /* [22] */
        unsigned int fifo_rd_int567 : 1; /* [23] */
        unsigned int fifo_rd_int568 : 1; /* [24] */
        unsigned int fifo_rd_int569 : 1; /* [25] */
        unsigned int fifo_rd_int570 : 1; /* [26] */
        unsigned int fifo_rd_int571 : 1; /* [27] */
        unsigned int fifo_rd_int572 : 1; /* [28] */
        unsigned int fifo_rd_int573 : 1; /* [29] */
        unsigned int fifo_rd_int574 : 1; /* [30] */
        unsigned int fifo_rd_int575 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_rd_int17;

typedef union {
    struct {
        unsigned int fifo_rd_int576 : 1; /* [0] */
        unsigned int fifo_rd_int577 : 1; /* [1] */
        unsigned int fifo_rd_int578 : 1; /* [2] */
        unsigned int fifo_rd_int579 : 1; /* [3] */
        unsigned int fifo_rd_int580 : 1; /* [4] */
        unsigned int fifo_rd_int581 : 1; /* [5] */
        unsigned int fifo_rd_int582 : 1; /* [6] */
        unsigned int fifo_rd_int583 : 1; /* [7] */
        unsigned int fifo_rd_int584 : 1; /* [8] */
        unsigned int fifo_rd_int585 : 1; /* [9] */
        unsigned int fifo_rd_int586 : 1; /* [10] */
        unsigned int fifo_rd_int587 : 1; /* [11] */
        unsigned int fifo_rd_int588 : 1; /* [12] */
        unsigned int fifo_rd_int589 : 1; /* [13] */
        unsigned int fifo_rd_int590 : 1; /* [14] */
        unsigned int fifo_rd_int591 : 1; /* [15] */
        unsigned int fifo_rd_int592 : 1; /* [16] */
        unsigned int fifo_rd_int593 : 1; /* [17] */
        unsigned int fifo_rd_int594 : 1; /* [18] */
        unsigned int fifo_rd_int595 : 1; /* [19] */
        unsigned int fifo_rd_int596 : 1; /* [20] */
        unsigned int fifo_rd_int597 : 1; /* [21] */
        unsigned int fifo_rd_int598 : 1; /* [22] */
        unsigned int fifo_rd_int599 : 1; /* [23] */
        unsigned int fifo_rd_int600 : 1; /* [24] */
        unsigned int fifo_rd_int601 : 1; /* [25] */
        unsigned int fifo_rd_int602 : 1; /* [26] */
        unsigned int fifo_rd_int603 : 1; /* [27] */
        unsigned int fifo_rd_int604 : 1; /* [28] */
        unsigned int fifo_rd_int605 : 1; /* [29] */
        unsigned int fifo_rd_int606 : 1; /* [30] */
        unsigned int fifo_rd_int607 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_tpu_fifo_rd_int18;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_type : 2; /* [1:0] */
        unsigned int mem_ecc_mbit_int_type : 2; /* [3:2] */
        unsigned int fifo_rd_int_type : 2;      /* [5:4] */
        unsigned int fifo_wr_int_type : 2;      /* [7:6] */
        unsigned int fsm_hbeat_int_type : 2;    /* [9:8] */
        unsigned int axi_err_int_type : 2;      /* [11:10] */
        unsigned int rsv_7 : 20;                /* [31:12] */
    } bits;

    unsigned int value;
} u_tpu_pf_int_type0;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_ce : 1; /* [0] */
        unsigned int mem_ecc_mbit_int_ce : 1; /* [1] */
        unsigned int fifo_rd_int_ce : 1;      /* [2] */
        unsigned int fifo_wr_int_ce : 1;      /* [3] */
        unsigned int fsm_hbeat_int_ce : 1;    /* [4] */
        unsigned int axi_err_int_ce : 1;      /* [5] */
        unsigned int rsv_8 : 26;              /* [31:6] */
    } bits;

    unsigned int value;
} u_tpu_int_ce0;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_nfe : 1; /* [0] */
        unsigned int mem_ecc_mbit_int_nfe : 1; /* [1] */
        unsigned int fifo_rd_int_nfe : 1;      /* [2] */
        unsigned int fifo_wr_int_nfe : 1;      /* [3] */
        unsigned int fsm_hbeat_int_nfe : 1;    /* [4] */
        unsigned int axi_err_int_nfe : 1;      /* [5] */
        unsigned int rsv_9 : 26;               /* [31:6] */
    } bits;

    unsigned int value;
} u_tpu_int_nfe0;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_fe : 1; /* [0] */
        unsigned int mem_ecc_mbit_int_fe : 1; /* [1] */
        unsigned int fifo_rd_int_fe : 1;      /* [2] */
        unsigned int fifo_wr_int_fe : 1;      /* [3] */
        unsigned int fsm_hbeat_int_fe : 1;    /* [4] */
        unsigned int axi_err_int_fe : 1;      /* [5] */
        unsigned int rsv_10 : 26;             /* [31:6] */
    } bits;

    unsigned int value;
} u_tpu_int_fe0;

typedef union {
    struct {
        unsigned int mem_init_start : 1; /* [0] */
        unsigned int rsv_11 : 31;        /* [31:1] */
    } bits;

    unsigned int value;
} u_tpu_mem_init_start;

typedef union {
    struct {
        unsigned int mem_init_done : 1; /* [0] */
        unsigned int rsv_12 : 31;       /* [31:1] */
    } bits;

    unsigned int value;
} u_tpu_mem_init_done;

typedef union {
    struct {
        unsigned int cfg_tp_mem_timing_ctrl : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tpu_mem_ctrl_tptmod;

typedef union {
    struct {
        unsigned int cfg_sp_mem_timing_ctrl : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tpu_mem_ctrl_sptmod;

typedef union {
    struct {
        unsigned int cfg_mem_power_ctrl : 8; /* [7:0] */
        unsigned int rsv_13 : 24;            /* [31:8] */
    } bits;

    unsigned int value;
} u_tpu_mem_ctrl_power_ctrl;

typedef union {
    struct {
        unsigned int cfg_mem_ecc_bypass : 1; /* [0] */
        unsigned int rsv_14 : 31;            /* [31:1] */
    } bits;

    unsigned int value;
} u_tpu_mem_ecc_bypass;

typedef union {
    struct {
        unsigned int cfg_tx_mem_err_thd : 8; /* [7:0] */
        unsigned int rsv_15 : 24;            /* [31:8] */
    } bits;

    unsigned int value;
} u_tpu_mem_serr_thd;

typedef union {
    struct {
        unsigned int cfg_tpu_eco0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tpu_eco0;

typedef union {
    struct {
        unsigned int cfg_tpu_eco1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tpu_eco1;

typedef union {
    struct {
        unsigned int cfg_tpu_fsm_timeout : 16; /* [15:0] */
        unsigned int rsv_16 : 16;              /* [31:16] */
    } bits;

    unsigned int value;
} u_tpu_fsm_heat_timeout;

typedef union {
    struct {
        unsigned int cfg_cnt_clr_ce : 1; /* [0] */
        unsigned int rsv_17 : 31;        /* [31:1] */
    } bits;

    unsigned int value;
} u_tpu_cnt_clr_ce;

typedef union {
    struct {
        unsigned int tpu_ns_lock : 16; /* [15:0] */
        unsigned int rsv_18 : 16;      /* [31:16] */
    } bits;

    unsigned int value;
} u_tpu_ns_reg;

typedef union {
    struct {
        unsigned int tpu_mecc_process : 1; /* [0] */
        unsigned int rsv_19 : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_tpu_mecc_process_sel;

typedef union {
    struct {
        unsigned int cfg_tpu_auto_gating_enable : 1; /* [0] */
        unsigned int rsv_20 : 15;                    /* [15:1] */
        unsigned int cfg_tpu_gating_sts_port : 8;    /* [23:16] */
        unsigned int rsv_21 : 8;                     /* [31:24] */
    } bits;

    unsigned int value;
} u_tpu_clk_gating_ctrl;

typedef union {
    struct {
        unsigned int cfg_tpu_gating_win : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tpu_gating_window;

typedef union {
    struct {
        unsigned int tpu_gating_sts : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tpu_gating_sts;

typedef union {
    struct {
        unsigned int cfg_tpu_max_pktlen : 16; /* [15:0] */
        unsigned int rsv_22 : 16;             /* [31:16] */
    } bits;

    unsigned int value;
} u_tpu_max_pktlen;

typedef union {
    struct {
        unsigned int cfg_tpu_pkt_padding_en : 1; /* [0] */
        unsigned int rsv_23 : 31;                /* [31:1] */
    } bits;

    unsigned int value;
} u_tpu_min_pktlen_padding_en;

typedef union {
    struct {
        unsigned int cfg_tx_wl_cnt : 9; /* [8:0] */
        unsigned int rsv_24 : 23;       /* [31:9] */
    } bits;

    unsigned int value;
} u_tpu_tx_wl_cnt;

typedef union {
    struct {
        unsigned int cfg_tpu_rd_pkt_outstand_num : 7; /* [6:0] */
        unsigned int rsv_25 : 25;                     /* [31:7] */
    } bits;

    unsigned int value;
} u_tpu_outstand_ctrl;

typedef union {
    struct {
        unsigned int cfg_pkt_rd_burst_ctrl_en : 1; /* [0] */
        unsigned int rsv_26 : 31;                  /* [31:1] */
    } bits;

    unsigned int value;
} u_tpu_pkt_rd_burst_ctrl_0;

typedef union {
    struct {
        unsigned int cfg_pkt_rd_burst_ctrl_wl : 10; /* [9:0] */
        unsigned int rsv_27 : 22;                   /* [31:10] */
    } bits;

    unsigned int value;
} u_tpu_pkt_rd_burst_ctrl_1;

typedef union {
    struct {
        unsigned int cfg_ply_store_aful_gap : 8; /* [7:0] */
        unsigned int rsv_28 : 24;                /* [31:8] */
    } bits;

    unsigned int value;
} u_tpu_ply_store_gap;

typedef union {
    struct {
        unsigned int cfg_prebd_ncge_chl_thr : 6; /* [5:0] */
        unsigned int rsv_29 : 2;                 /* [7:6] */
        unsigned int cfg_prebd_cge_chl_thr : 6;  /* [13:8] */
        unsigned int rsv_30 : 2;                 /* [15:14] */
        unsigned int cfg_tpu_prebd_aful_gap : 6; /* [21:16] */
        unsigned int rsv_31 : 10;                /* [31:22] */
    } bits;

    unsigned int value;
} u_tpu_prebd_fifo_thr;

typedef union {
    struct {
        unsigned int cfg_tnl_ply_fifo_depth : 8; /* [7:0] */
        unsigned int rsv_32 : 24;                /* [31:8] */
    } bits;

    unsigned int value;
} u_tpu_tnl_fifo_depth;

typedef union {
    struct {
        unsigned int cfg_bd_endian_ctrl : 2; /* [1:0] */
        unsigned int rsv_33 : 30;            /* [31:2] */
    } bits;

    unsigned int value;
} u_tpu_endian_ctrl;

typedef union {
    struct {
        unsigned int cfg_tso_oip_id_type : 1; /* [0] */
        unsigned int rsv_34 : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_tpu_tso_oip_id_type;

typedef union {
    struct {
        unsigned int cfg_tso_flag_msk_fst : 12; /* [11:0] */
        unsigned int rsv_35 : 20;               /* [31:12] */
    } bits;

    unsigned int value;
} u_tpu_tso_flag_msk_fst;

typedef union {
    struct {
        unsigned int cfg_tso_flag_msk_mid : 12; /* [11:0] */
        unsigned int rsv_36 : 20;               /* [31:12] */
    } bits;

    unsigned int value;
} u_tpu_tso_flag_msk_mid;

typedef union {
    struct {
        unsigned int cfg_tso_flag_msk_last : 12; /* [11:0] */
        unsigned int rsv_37 : 20;                /* [31:12] */
    } bits;

    unsigned int value;
} u_tpu_tso_flag_msk_last;

typedef union {
    struct {
        unsigned int cfg_l3cks_cs_fifo_aful_gap : 4;  /* [3:0] */
        unsigned int rsv_38 : 4;                      /* [7:4] */
        unsigned int cfg_l3cks_pld_fifo_aful_gap : 5; /* [12:8] */
        unsigned int rsv_39 : 19;                     /* [31:13] */
    } bits;

    unsigned int value;
} u_tpu_l3cks_fifo_gap;

typedef union {
    struct {
        unsigned int cfg_rb_wb_fifo_aful_gap : 5; /* [4:0] */
        unsigned int rsv_40 : 27;                 /* [31:5] */
    } bits;

    unsigned int value;
} u_tpu_rb_wb_fifo_agul_gap;

typedef union {
    struct {
        unsigned int cfg_rb_tc_bd_wb_weight : 8; /* [7:0] */
        unsigned int rsv_41 : 24;                /* [31:8] */
    } bits;

    unsigned int value;
} u_tpu_rb_tc_weight;

typedef union {
    struct {
        unsigned int cfg_ddr_pkt_streamid : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tpu_ddr_pkt_smmu_user_strmid;

typedef union {
    struct {
        unsigned int cfg_ddr_bd_sset : 1;  /* [0] */
        unsigned int rsv_42 : 3;           /* [3:1] */
        unsigned int cfg_ddr_pkt_sset : 1; /* [4] */
        unsigned int rsv_43 : 27;          /* [31:5] */
    } bits;

    unsigned int value;
} u_tpu_smmu_user_sset;

typedef union {
    struct {
        unsigned int cfg_ddr_r_fa : 1;                 /* [0] */
        unsigned int cfg_ddr_r_bd_fna : 1;             /* [1] */
        unsigned int cfg_ddr_r_ply_fna : 1;            /* [2] */
        unsigned int cfg_ddr_r_rinvld : 1;             /* [3] */
        unsigned int cfg_ddr_r_bd_type : 4;            /* [7:4] */
        unsigned int cfg_ddr_r_ply_type : 4;           /* [11:8] */
        unsigned int rsv_44 : 3;                       /* [14:12] */
        unsigned int cfg_ddr_r_snpattr : 1;            /* [15] */
        unsigned int cfg_ddr_w_fa : 1;                 /* [16] */
        unsigned int cfg_ddr_w_bd_fna : 1;             /* [17] */
        unsigned int cfg_ddr_w_ply_fna : 1;            /* [18] */
        unsigned int cfg_ddr_w_force_partial_ctrl : 1; /* [19] */
        unsigned int cfg_ddr_w_bd_type : 4;            /* [23:20] */
        unsigned int cfg_ddr_w_ply_type : 4;           /* [27:24] */
        unsigned int rsv_45 : 3;                       /* [30:28] */
        unsigned int cfg_ddr_w_snpattr : 1;            /* [31] */
    } bits;

    unsigned int value;
} u_tpu_ddr_smmu_user;

typedef union {
    struct {
        unsigned int cfg_ddr_partid : 8;        /* [7:0] */
        unsigned int cfg_ddr_pmg : 2;           /* [9:8] */
        unsigned int rsv_46 : 2;                /* [11:10] */
        unsigned int cfg_ddr_new_sub_atype : 4; /* [15:12] */
        unsigned int cfg_ddr_new_atype : 4;     /* [19:16] */
        unsigned int rsv_47 : 12;               /* [31:20] */
    } bits;

    unsigned int value;
} u_tpu_ddr_smmu_user1;

typedef union {
    struct {
        unsigned int cfg_user_fa_sel : 1;    /* [0] */
        unsigned int cfg_user_lock_sel : 1;  /* [1] */
        unsigned int rsv_48 : 2;             /* [3:2] */
        unsigned int cfg_user_head_type : 2; /* [5:4] */
        unsigned int rsv_49 : 26;            /* [31:6] */
    } bits;

    unsigned int value;
} u_tpu_ddr_smmu_user2;

typedef union {
    struct {
        unsigned int cfg_hac_aw_qos : 4; /* [3:0] */
        unsigned int cfg_hac_ar_qos : 4; /* [7:4] */
        unsigned int cfg_ddr_aw_qos : 4; /* [11:8] */
        unsigned int cfg_ddr_ar_qos : 4; /* [15:12] */
        unsigned int rsv_50 : 16;        /* [31:16] */
    } bits;

    unsigned int value;
} u_tpu_cfg_bus_qos;

typedef union {
    struct {
        unsigned int cfg_hac_aw_cache : 4; /* [3:0] */
        unsigned int cfg_hac_ar_cache : 4; /* [7:4] */
        unsigned int cfg_ddr_aw_cache : 4; /* [11:8] */
        unsigned int cfg_ddr_ar_cache : 4; /* [15:12] */
        unsigned int rsv_51 : 16;          /* [31:16] */
    } bits;

    unsigned int value;
} u_tpu_cfg_bus_cache;

typedef union {
    struct {
        unsigned int rcb_add_pkt_cnt : 48; /* [47:0] */
        unsigned int rsv_52 : 16;          /* [63:48] */
    } bits;

    unsigned int value;
} u_tpu_rcb_add_pkt_cnt;

typedef union {
    struct {
        unsigned int tx_send_frag_cnt : 48; /* [47:0] */
        unsigned int rsv_53 : 16;           /* [63:48] */
    } bits;

    unsigned int value;
} u_tpu_send_frag_cnt;

typedef union {
    struct {
        unsigned int tx_send_pkt_cnt : 48; /* [47:0] */
        unsigned int rsv_54 : 16;          /* [63:48] */
    } bits;

    unsigned int value;
} u_tpu_send_pkt_cnt;

typedef union {
    struct {
        unsigned int tpu_mem_sbit_err_cnt : 16; /* [15:0] */
        unsigned int rsv_55 : 16;               /* [31:16] */
    } bits;

    unsigned int value;
} u_tpu_mem_sbit_err_cnt;

typedef union {
    struct {
        unsigned int tpu_mem_mbit_err_cnt : 16; /* [15:0] */
        unsigned int rsv_56 : 16;               /* [31:16] */
    } bits;

    unsigned int value;
} u_tpu_mem_mbit_err_cnt;

typedef union {
    struct {
        unsigned int ssu_tpu_tx_bp : 1;    /* [0] */
        unsigned int rsv_57 : 7;           /* [7:1] */
        unsigned int tx_prebd_buf_bp : 1;  /* [8] */
        unsigned int rsv_58 : 7;           /* [15:9] */
        unsigned int pktr_ply_cnt_bp : 1;  /* [16] */
        unsigned int rsv_59 : 7;           /* [23:17] */
        unsigned int pktr_ar_trans_bp : 1; /* [24] */
        unsigned int rsv_60 : 7;           /* [31:25] */
    } bits;

    unsigned int value;
} u_ssu_tpu_bp_status;

typedef union {
    struct {
        unsigned int ssu_tpu_tx_tnl_bp : 4; /* [3:0] */
        unsigned int rsv_61 : 28;           /* [31:4] */
    } bits;

    unsigned int value;
} u_ssu_tpu_tnl_bp_status;

typedef union {
    struct {
        unsigned int tpu_rd_wb_bd_rdy : 4; /* [3:0] */
        unsigned int rsv_62 : 28;          /* [31:4] */
    } bits;

    unsigned int value;
} u_tpu_rb_wb_bd_rdy;

typedef union {
    struct {
        unsigned int tnl_rd_pkt_bp : 4; /* [3:0] */
        unsigned int rsv_63 : 28;       /* [31:4] */
    } bits;

    unsigned int value;
} u_tpu_rdpkt_bp_status;

typedef union {
    struct {
        unsigned int tnl_prebd_buf_bp : 4; /* [3:0] */
        unsigned int rsv_64 : 28;          /* [31:4] */
    } bits;

    unsigned int value;
} u_tpu_prebd_buf_bp_status;

typedef union {
    struct {
        unsigned int tnl_beat_cnt_bp : 4; /* [3:0] */
        unsigned int rsv_65 : 28;         /* [31:4] */
    } bits;

    unsigned int value;
} u_tpu_tnl_beat_cnt_bp;

typedef union {
    struct {
        unsigned int tnl_pkt_cnt_bp : 4; /* [3:0] */
        unsigned int rsv_66 : 28;        /* [31:4] */
    } bits;

    unsigned int value;
} u_tpu_tnl_pkt_cnt_bp;

typedef union {
    struct {
        unsigned int mac_rb_bd_wb_bp : 4; /* [3:0] */
        unsigned int rsv_67 : 28;         /* [31:4] */
    } bits;

    unsigned int value;
} u_tpu_rb_bd_wb_bp;

typedef union {
    struct {
        unsigned int tpu_tnl_datapath_empty : 4; /* [3:0] */
        unsigned int rsv_68 : 28;                /* [31:4] */
    } bits;

    unsigned int value;
} u_tpu_tnl_empty_status;

typedef union {
    struct {
        unsigned int tpu_datapath_empty : 1; /* [0] */
        unsigned int rsv_69 : 31;            /* [31:1] */
    } bits;

    unsigned int value;
} u_tpu_datapath_empty_status;

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
} u_tpu_fifo_dfx0;

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
} u_tpu_fifo_dfx1;

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
} u_tpu_fifo_dfx2;

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
} u_tpu_fifo_dfx3;

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
} u_tpu_fifo_dfx4;

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
} u_tpu_fifo_dfx5;

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
} u_tpu_fifo_dfx6;

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
} u_tpu_fifo_dfx7;

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
} u_tpu_fifo_dfx8;

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
} u_tpu_fifo_dfx9;

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
} u_tpu_fifo_dfx10;

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
} u_tpu_fifo_dfx11;

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
} u_tpu_fifo_dfx12;

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
} u_tpu_fifo_dfx13;

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
} u_tpu_fifo_dfx14;

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
} u_tpu_fifo_dfx15;

typedef union {
    struct {
        unsigned int fifo_dfx256 : 2; /* [1:0] */
        unsigned int fifo_dfx257 : 2; /* [3:2] */
        unsigned int fifo_dfx258 : 2; /* [5:4] */
        unsigned int fifo_dfx259 : 2; /* [7:6] */
        unsigned int fifo_dfx260 : 2; /* [9:8] */
        unsigned int fifo_dfx261 : 2; /* [11:10] */
        unsigned int fifo_dfx262 : 2; /* [13:12] */
        unsigned int fifo_dfx263 : 2; /* [15:14] */
        unsigned int fifo_dfx264 : 2; /* [17:16] */
        unsigned int fifo_dfx265 : 2; /* [19:18] */
        unsigned int fifo_dfx266 : 2; /* [21:20] */
        unsigned int fifo_dfx267 : 2; /* [23:22] */
        unsigned int fifo_dfx268 : 2; /* [25:24] */
        unsigned int fifo_dfx269 : 2; /* [27:26] */
        unsigned int fifo_dfx270 : 2; /* [29:28] */
        unsigned int fifo_dfx271 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx16;

typedef union {
    struct {
        unsigned int fifo_dfx272 : 2; /* [1:0] */
        unsigned int fifo_dfx273 : 2; /* [3:2] */
        unsigned int fifo_dfx274 : 2; /* [5:4] */
        unsigned int fifo_dfx275 : 2; /* [7:6] */
        unsigned int fifo_dfx276 : 2; /* [9:8] */
        unsigned int fifo_dfx277 : 2; /* [11:10] */
        unsigned int fifo_dfx278 : 2; /* [13:12] */
        unsigned int fifo_dfx279 : 2; /* [15:14] */
        unsigned int fifo_dfx280 : 2; /* [17:16] */
        unsigned int fifo_dfx281 : 2; /* [19:18] */
        unsigned int fifo_dfx282 : 2; /* [21:20] */
        unsigned int fifo_dfx283 : 2; /* [23:22] */
        unsigned int fifo_dfx284 : 2; /* [25:24] */
        unsigned int fifo_dfx285 : 2; /* [27:26] */
        unsigned int fifo_dfx286 : 2; /* [29:28] */
        unsigned int fifo_dfx287 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx17;

typedef union {
    struct {
        unsigned int fifo_dfx288 : 2; /* [1:0] */
        unsigned int fifo_dfx289 : 2; /* [3:2] */
        unsigned int fifo_dfx290 : 2; /* [5:4] */
        unsigned int fifo_dfx291 : 2; /* [7:6] */
        unsigned int fifo_dfx292 : 2; /* [9:8] */
        unsigned int fifo_dfx293 : 2; /* [11:10] */
        unsigned int fifo_dfx294 : 2; /* [13:12] */
        unsigned int fifo_dfx295 : 2; /* [15:14] */
        unsigned int fifo_dfx296 : 2; /* [17:16] */
        unsigned int fifo_dfx297 : 2; /* [19:18] */
        unsigned int fifo_dfx298 : 2; /* [21:20] */
        unsigned int fifo_dfx299 : 2; /* [23:22] */
        unsigned int fifo_dfx300 : 2; /* [25:24] */
        unsigned int fifo_dfx301 : 2; /* [27:26] */
        unsigned int fifo_dfx302 : 2; /* [29:28] */
        unsigned int fifo_dfx303 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx18;

typedef union {
    struct {
        unsigned int fifo_dfx304 : 2; /* [1:0] */
        unsigned int fifo_dfx305 : 2; /* [3:2] */
        unsigned int fifo_dfx306 : 2; /* [5:4] */
        unsigned int fifo_dfx307 : 2; /* [7:6] */
        unsigned int fifo_dfx308 : 2; /* [9:8] */
        unsigned int fifo_dfx309 : 2; /* [11:10] */
        unsigned int fifo_dfx310 : 2; /* [13:12] */
        unsigned int fifo_dfx311 : 2; /* [15:14] */
        unsigned int fifo_dfx312 : 2; /* [17:16] */
        unsigned int fifo_dfx313 : 2; /* [19:18] */
        unsigned int fifo_dfx314 : 2; /* [21:20] */
        unsigned int fifo_dfx315 : 2; /* [23:22] */
        unsigned int fifo_dfx316 : 2; /* [25:24] */
        unsigned int fifo_dfx317 : 2; /* [27:26] */
        unsigned int fifo_dfx318 : 2; /* [29:28] */
        unsigned int fifo_dfx319 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx19;

typedef union {
    struct {
        unsigned int fifo_dfx320 : 2; /* [1:0] */
        unsigned int fifo_dfx321 : 2; /* [3:2] */
        unsigned int fifo_dfx322 : 2; /* [5:4] */
        unsigned int fifo_dfx323 : 2; /* [7:6] */
        unsigned int fifo_dfx324 : 2; /* [9:8] */
        unsigned int fifo_dfx325 : 2; /* [11:10] */
        unsigned int fifo_dfx326 : 2; /* [13:12] */
        unsigned int fifo_dfx327 : 2; /* [15:14] */
        unsigned int fifo_dfx328 : 2; /* [17:16] */
        unsigned int fifo_dfx329 : 2; /* [19:18] */
        unsigned int fifo_dfx330 : 2; /* [21:20] */
        unsigned int fifo_dfx331 : 2; /* [23:22] */
        unsigned int fifo_dfx332 : 2; /* [25:24] */
        unsigned int fifo_dfx333 : 2; /* [27:26] */
        unsigned int fifo_dfx334 : 2; /* [29:28] */
        unsigned int fifo_dfx335 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx20;

typedef union {
    struct {
        unsigned int fifo_dfx336 : 2; /* [1:0] */
        unsigned int fifo_dfx337 : 2; /* [3:2] */
        unsigned int fifo_dfx338 : 2; /* [5:4] */
        unsigned int fifo_dfx339 : 2; /* [7:6] */
        unsigned int fifo_dfx340 : 2; /* [9:8] */
        unsigned int fifo_dfx341 : 2; /* [11:10] */
        unsigned int fifo_dfx342 : 2; /* [13:12] */
        unsigned int fifo_dfx343 : 2; /* [15:14] */
        unsigned int fifo_dfx344 : 2; /* [17:16] */
        unsigned int fifo_dfx345 : 2; /* [19:18] */
        unsigned int fifo_dfx346 : 2; /* [21:20] */
        unsigned int fifo_dfx347 : 2; /* [23:22] */
        unsigned int fifo_dfx348 : 2; /* [25:24] */
        unsigned int fifo_dfx349 : 2; /* [27:26] */
        unsigned int fifo_dfx350 : 2; /* [29:28] */
        unsigned int fifo_dfx351 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx21;

typedef union {
    struct {
        unsigned int fifo_dfx352 : 2; /* [1:0] */
        unsigned int fifo_dfx353 : 2; /* [3:2] */
        unsigned int fifo_dfx354 : 2; /* [5:4] */
        unsigned int fifo_dfx355 : 2; /* [7:6] */
        unsigned int fifo_dfx356 : 2; /* [9:8] */
        unsigned int fifo_dfx357 : 2; /* [11:10] */
        unsigned int fifo_dfx358 : 2; /* [13:12] */
        unsigned int fifo_dfx359 : 2; /* [15:14] */
        unsigned int fifo_dfx360 : 2; /* [17:16] */
        unsigned int fifo_dfx361 : 2; /* [19:18] */
        unsigned int fifo_dfx362 : 2; /* [21:20] */
        unsigned int fifo_dfx363 : 2; /* [23:22] */
        unsigned int fifo_dfx364 : 2; /* [25:24] */
        unsigned int fifo_dfx365 : 2; /* [27:26] */
        unsigned int fifo_dfx366 : 2; /* [29:28] */
        unsigned int fifo_dfx367 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx22;

typedef union {
    struct {
        unsigned int fifo_dfx368 : 2; /* [1:0] */
        unsigned int fifo_dfx369 : 2; /* [3:2] */
        unsigned int fifo_dfx370 : 2; /* [5:4] */
        unsigned int fifo_dfx371 : 2; /* [7:6] */
        unsigned int fifo_dfx372 : 2; /* [9:8] */
        unsigned int fifo_dfx373 : 2; /* [11:10] */
        unsigned int fifo_dfx374 : 2; /* [13:12] */
        unsigned int fifo_dfx375 : 2; /* [15:14] */
        unsigned int fifo_dfx376 : 2; /* [17:16] */
        unsigned int fifo_dfx377 : 2; /* [19:18] */
        unsigned int fifo_dfx378 : 2; /* [21:20] */
        unsigned int fifo_dfx379 : 2; /* [23:22] */
        unsigned int fifo_dfx380 : 2; /* [25:24] */
        unsigned int fifo_dfx381 : 2; /* [27:26] */
        unsigned int fifo_dfx382 : 2; /* [29:28] */
        unsigned int fifo_dfx383 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx23;

typedef union {
    struct {
        unsigned int fifo_dfx384 : 2; /* [1:0] */
        unsigned int fifo_dfx385 : 2; /* [3:2] */
        unsigned int fifo_dfx386 : 2; /* [5:4] */
        unsigned int fifo_dfx387 : 2; /* [7:6] */
        unsigned int fifo_dfx388 : 2; /* [9:8] */
        unsigned int fifo_dfx389 : 2; /* [11:10] */
        unsigned int fifo_dfx390 : 2; /* [13:12] */
        unsigned int fifo_dfx391 : 2; /* [15:14] */
        unsigned int fifo_dfx392 : 2; /* [17:16] */
        unsigned int fifo_dfx393 : 2; /* [19:18] */
        unsigned int fifo_dfx394 : 2; /* [21:20] */
        unsigned int fifo_dfx395 : 2; /* [23:22] */
        unsigned int fifo_dfx396 : 2; /* [25:24] */
        unsigned int fifo_dfx397 : 2; /* [27:26] */
        unsigned int fifo_dfx398 : 2; /* [29:28] */
        unsigned int fifo_dfx399 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx24;

typedef union {
    struct {
        unsigned int fifo_dfx400 : 2; /* [1:0] */
        unsigned int fifo_dfx401 : 2; /* [3:2] */
        unsigned int fifo_dfx402 : 2; /* [5:4] */
        unsigned int fifo_dfx403 : 2; /* [7:6] */
        unsigned int fifo_dfx404 : 2; /* [9:8] */
        unsigned int fifo_dfx405 : 2; /* [11:10] */
        unsigned int fifo_dfx406 : 2; /* [13:12] */
        unsigned int fifo_dfx407 : 2; /* [15:14] */
        unsigned int fifo_dfx408 : 2; /* [17:16] */
        unsigned int fifo_dfx409 : 2; /* [19:18] */
        unsigned int fifo_dfx410 : 2; /* [21:20] */
        unsigned int fifo_dfx411 : 2; /* [23:22] */
        unsigned int fifo_dfx412 : 2; /* [25:24] */
        unsigned int fifo_dfx413 : 2; /* [27:26] */
        unsigned int fifo_dfx414 : 2; /* [29:28] */
        unsigned int fifo_dfx415 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx25;

typedef union {
    struct {
        unsigned int fifo_dfx416 : 2; /* [1:0] */
        unsigned int fifo_dfx417 : 2; /* [3:2] */
        unsigned int fifo_dfx418 : 2; /* [5:4] */
        unsigned int fifo_dfx419 : 2; /* [7:6] */
        unsigned int fifo_dfx420 : 2; /* [9:8] */
        unsigned int fifo_dfx421 : 2; /* [11:10] */
        unsigned int fifo_dfx422 : 2; /* [13:12] */
        unsigned int fifo_dfx423 : 2; /* [15:14] */
        unsigned int fifo_dfx424 : 2; /* [17:16] */
        unsigned int fifo_dfx425 : 2; /* [19:18] */
        unsigned int fifo_dfx426 : 2; /* [21:20] */
        unsigned int fifo_dfx427 : 2; /* [23:22] */
        unsigned int fifo_dfx428 : 2; /* [25:24] */
        unsigned int fifo_dfx429 : 2; /* [27:26] */
        unsigned int fifo_dfx430 : 2; /* [29:28] */
        unsigned int fifo_dfx431 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx26;

typedef union {
    struct {
        unsigned int fifo_dfx432 : 2; /* [1:0] */
        unsigned int fifo_dfx433 : 2; /* [3:2] */
        unsigned int fifo_dfx434 : 2; /* [5:4] */
        unsigned int fifo_dfx435 : 2; /* [7:6] */
        unsigned int fifo_dfx436 : 2; /* [9:8] */
        unsigned int fifo_dfx437 : 2; /* [11:10] */
        unsigned int fifo_dfx438 : 2; /* [13:12] */
        unsigned int fifo_dfx439 : 2; /* [15:14] */
        unsigned int fifo_dfx440 : 2; /* [17:16] */
        unsigned int fifo_dfx441 : 2; /* [19:18] */
        unsigned int fifo_dfx442 : 2; /* [21:20] */
        unsigned int fifo_dfx443 : 2; /* [23:22] */
        unsigned int fifo_dfx444 : 2; /* [25:24] */
        unsigned int fifo_dfx445 : 2; /* [27:26] */
        unsigned int fifo_dfx446 : 2; /* [29:28] */
        unsigned int fifo_dfx447 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx27;

typedef union {
    struct {
        unsigned int fifo_dfx448 : 2; /* [1:0] */
        unsigned int fifo_dfx449 : 2; /* [3:2] */
        unsigned int fifo_dfx450 : 2; /* [5:4] */
        unsigned int fifo_dfx451 : 2; /* [7:6] */
        unsigned int fifo_dfx452 : 2; /* [9:8] */
        unsigned int fifo_dfx453 : 2; /* [11:10] */
        unsigned int fifo_dfx454 : 2; /* [13:12] */
        unsigned int fifo_dfx455 : 2; /* [15:14] */
        unsigned int fifo_dfx456 : 2; /* [17:16] */
        unsigned int fifo_dfx457 : 2; /* [19:18] */
        unsigned int fifo_dfx458 : 2; /* [21:20] */
        unsigned int fifo_dfx459 : 2; /* [23:22] */
        unsigned int fifo_dfx460 : 2; /* [25:24] */
        unsigned int fifo_dfx461 : 2; /* [27:26] */
        unsigned int fifo_dfx462 : 2; /* [29:28] */
        unsigned int fifo_dfx463 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx28;

typedef union {
    struct {
        unsigned int fifo_dfx464 : 2; /* [1:0] */
        unsigned int fifo_dfx465 : 2; /* [3:2] */
        unsigned int fifo_dfx466 : 2; /* [5:4] */
        unsigned int fifo_dfx467 : 2; /* [7:6] */
        unsigned int fifo_dfx468 : 2; /* [9:8] */
        unsigned int fifo_dfx469 : 2; /* [11:10] */
        unsigned int fifo_dfx470 : 2; /* [13:12] */
        unsigned int fifo_dfx471 : 2; /* [15:14] */
        unsigned int fifo_dfx472 : 2; /* [17:16] */
        unsigned int fifo_dfx473 : 2; /* [19:18] */
        unsigned int fifo_dfx474 : 2; /* [21:20] */
        unsigned int fifo_dfx475 : 2; /* [23:22] */
        unsigned int fifo_dfx476 : 2; /* [25:24] */
        unsigned int fifo_dfx477 : 2; /* [27:26] */
        unsigned int fifo_dfx478 : 2; /* [29:28] */
        unsigned int fifo_dfx479 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx29;

typedef union {
    struct {
        unsigned int fifo_dfx480 : 2; /* [1:0] */
        unsigned int fifo_dfx481 : 2; /* [3:2] */
        unsigned int fifo_dfx482 : 2; /* [5:4] */
        unsigned int fifo_dfx483 : 2; /* [7:6] */
        unsigned int fifo_dfx484 : 2; /* [9:8] */
        unsigned int fifo_dfx485 : 2; /* [11:10] */
        unsigned int fifo_dfx486 : 2; /* [13:12] */
        unsigned int fifo_dfx487 : 2; /* [15:14] */
        unsigned int fifo_dfx488 : 2; /* [17:16] */
        unsigned int fifo_dfx489 : 2; /* [19:18] */
        unsigned int fifo_dfx490 : 2; /* [21:20] */
        unsigned int fifo_dfx491 : 2; /* [23:22] */
        unsigned int fifo_dfx492 : 2; /* [25:24] */
        unsigned int fifo_dfx493 : 2; /* [27:26] */
        unsigned int fifo_dfx494 : 2; /* [29:28] */
        unsigned int fifo_dfx495 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx30;

typedef union {
    struct {
        unsigned int fifo_dfx496 : 2; /* [1:0] */
        unsigned int fifo_dfx497 : 2; /* [3:2] */
        unsigned int fifo_dfx498 : 2; /* [5:4] */
        unsigned int fifo_dfx499 : 2; /* [7:6] */
        unsigned int fifo_dfx500 : 2; /* [9:8] */
        unsigned int fifo_dfx501 : 2; /* [11:10] */
        unsigned int fifo_dfx502 : 2; /* [13:12] */
        unsigned int fifo_dfx503 : 2; /* [15:14] */
        unsigned int fifo_dfx504 : 2; /* [17:16] */
        unsigned int fifo_dfx505 : 2; /* [19:18] */
        unsigned int fifo_dfx506 : 2; /* [21:20] */
        unsigned int fifo_dfx507 : 2; /* [23:22] */
        unsigned int fifo_dfx508 : 2; /* [25:24] */
        unsigned int fifo_dfx509 : 2; /* [27:26] */
        unsigned int fifo_dfx510 : 2; /* [29:28] */
        unsigned int fifo_dfx511 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx31;

typedef union {
    struct {
        unsigned int fifo_dfx512 : 2; /* [1:0] */
        unsigned int fifo_dfx513 : 2; /* [3:2] */
        unsigned int fifo_dfx514 : 2; /* [5:4] */
        unsigned int fifo_dfx515 : 2; /* [7:6] */
        unsigned int fifo_dfx516 : 2; /* [9:8] */
        unsigned int fifo_dfx517 : 2; /* [11:10] */
        unsigned int fifo_dfx518 : 2; /* [13:12] */
        unsigned int fifo_dfx519 : 2; /* [15:14] */
        unsigned int fifo_dfx520 : 2; /* [17:16] */
        unsigned int fifo_dfx521 : 2; /* [19:18] */
        unsigned int fifo_dfx522 : 2; /* [21:20] */
        unsigned int fifo_dfx523 : 2; /* [23:22] */
        unsigned int fifo_dfx524 : 2; /* [25:24] */
        unsigned int fifo_dfx525 : 2; /* [27:26] */
        unsigned int fifo_dfx526 : 2; /* [29:28] */
        unsigned int fifo_dfx527 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx32;

typedef union {
    struct {
        unsigned int fifo_dfx528 : 2; /* [1:0] */
        unsigned int fifo_dfx529 : 2; /* [3:2] */
        unsigned int fifo_dfx530 : 2; /* [5:4] */
        unsigned int fifo_dfx531 : 2; /* [7:6] */
        unsigned int fifo_dfx532 : 2; /* [9:8] */
        unsigned int fifo_dfx533 : 2; /* [11:10] */
        unsigned int fifo_dfx534 : 2; /* [13:12] */
        unsigned int fifo_dfx535 : 2; /* [15:14] */
        unsigned int fifo_dfx536 : 2; /* [17:16] */
        unsigned int fifo_dfx537 : 2; /* [19:18] */
        unsigned int fifo_dfx538 : 2; /* [21:20] */
        unsigned int fifo_dfx539 : 2; /* [23:22] */
        unsigned int fifo_dfx540 : 2; /* [25:24] */
        unsigned int fifo_dfx541 : 2; /* [27:26] */
        unsigned int fifo_dfx542 : 2; /* [29:28] */
        unsigned int fifo_dfx543 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx33;

typedef union {
    struct {
        unsigned int fifo_dfx544 : 2; /* [1:0] */
        unsigned int fifo_dfx545 : 2; /* [3:2] */
        unsigned int fifo_dfx546 : 2; /* [5:4] */
        unsigned int fifo_dfx547 : 2; /* [7:6] */
        unsigned int fifo_dfx548 : 2; /* [9:8] */
        unsigned int fifo_dfx549 : 2; /* [11:10] */
        unsigned int fifo_dfx550 : 2; /* [13:12] */
        unsigned int fifo_dfx551 : 2; /* [15:14] */
        unsigned int fifo_dfx552 : 2; /* [17:16] */
        unsigned int fifo_dfx553 : 2; /* [19:18] */
        unsigned int fifo_dfx554 : 2; /* [21:20] */
        unsigned int fifo_dfx555 : 2; /* [23:22] */
        unsigned int fifo_dfx556 : 2; /* [25:24] */
        unsigned int fifo_dfx557 : 2; /* [27:26] */
        unsigned int fifo_dfx558 : 2; /* [29:28] */
        unsigned int fifo_dfx559 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx34;

typedef union {
    struct {
        unsigned int fifo_dfx560 : 2; /* [1:0] */
        unsigned int fifo_dfx561 : 2; /* [3:2] */
        unsigned int fifo_dfx562 : 2; /* [5:4] */
        unsigned int fifo_dfx563 : 2; /* [7:6] */
        unsigned int fifo_dfx564 : 2; /* [9:8] */
        unsigned int fifo_dfx565 : 2; /* [11:10] */
        unsigned int fifo_dfx566 : 2; /* [13:12] */
        unsigned int fifo_dfx567 : 2; /* [15:14] */
        unsigned int fifo_dfx568 : 2; /* [17:16] */
        unsigned int fifo_dfx569 : 2; /* [19:18] */
        unsigned int fifo_dfx570 : 2; /* [21:20] */
        unsigned int fifo_dfx571 : 2; /* [23:22] */
        unsigned int fifo_dfx572 : 2; /* [25:24] */
        unsigned int fifo_dfx573 : 2; /* [27:26] */
        unsigned int fifo_dfx574 : 2; /* [29:28] */
        unsigned int fifo_dfx575 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx35;

typedef union {
    struct {
        unsigned int fifo_dfx576 : 2; /* [1:0] */
        unsigned int fifo_dfx577 : 2; /* [3:2] */
        unsigned int fifo_dfx578 : 2; /* [5:4] */
        unsigned int fifo_dfx579 : 2; /* [7:6] */
        unsigned int fifo_dfx580 : 2; /* [9:8] */
        unsigned int fifo_dfx581 : 2; /* [11:10] */
        unsigned int fifo_dfx582 : 2; /* [13:12] */
        unsigned int fifo_dfx583 : 2; /* [15:14] */
        unsigned int fifo_dfx584 : 2; /* [17:16] */
        unsigned int fifo_dfx585 : 2; /* [19:18] */
        unsigned int fifo_dfx586 : 2; /* [21:20] */
        unsigned int fifo_dfx587 : 2; /* [23:22] */
        unsigned int fifo_dfx588 : 2; /* [25:24] */
        unsigned int fifo_dfx589 : 2; /* [27:26] */
        unsigned int fifo_dfx590 : 2; /* [29:28] */
        unsigned int fifo_dfx591 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx36;

typedef union {
    struct {
        unsigned int fifo_dfx592 : 2; /* [1:0] */
        unsigned int fifo_dfx593 : 2; /* [3:2] */
        unsigned int fifo_dfx594 : 2; /* [5:4] */
        unsigned int fifo_dfx595 : 2; /* [7:6] */
        unsigned int fifo_dfx596 : 2; /* [9:8] */
        unsigned int fifo_dfx597 : 2; /* [11:10] */
        unsigned int fifo_dfx598 : 2; /* [13:12] */
        unsigned int fifo_dfx599 : 2; /* [15:14] */
        unsigned int fifo_dfx600 : 2; /* [17:16] */
        unsigned int fifo_dfx601 : 2; /* [19:18] */
        unsigned int fifo_dfx602 : 2; /* [21:20] */
        unsigned int fifo_dfx603 : 2; /* [23:22] */
        unsigned int fifo_dfx604 : 2; /* [25:24] */
        unsigned int fifo_dfx605 : 2; /* [27:26] */
        unsigned int fifo_dfx606 : 2; /* [29:28] */
        unsigned int fifo_dfx607 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_tpu_fifo_dfx37;

typedef union {
    struct {
        unsigned int roc_tx_in_cnt : 48; /* [47:0] */
        unsigned int rsv_70 : 16;        /* [63:48] */
    } bits;

    unsigned int value;
} u_roce_tx_in_pkt_cnt;

typedef union {
    struct {
        unsigned int roc_tx_out_cnt : 48; /* [47:0] */
        unsigned int rsv_71 : 16;         /* [63:48] */
    } bits;

    unsigned int value;
} u_roce_tx_out_pkt_cnt;

typedef union {
    struct {
        unsigned int nic_tx_in_ar_cnt : 48; /* [47:0] */
        unsigned int rsv_72 : 16;           /* [63:48] */
    } bits;

    unsigned int value;
} u_nic_tx_in_ar_cnt;

typedef union {
    struct {
        unsigned int roc_tx_in_ar_cnt : 48; /* [47:0] */
        unsigned int rsv_73 : 16;           /* [63:48] */
    } bits;

    unsigned int value;
} u_roce_tx_in_ar_cnt;

typedef union {
    struct {
        unsigned int axi_data_nic_cnt : 48; /* [47:0] */
        unsigned int rsv_74 : 16;           /* [63:48] */
    } bits;

    unsigned int value;
} u_axi_data_nic_cnt;

typedef union {
    struct {
        unsigned int axi_data_roc_cnt : 48; /* [47:0] */
        unsigned int rsv_75 : 16;           /* [63:48] */
    } bits;

    unsigned int value;
} u_axi_data_roce_cnt;

typedef union {
    struct {
        unsigned int txdma_ar_cnt_m0 : 48; /* [47:0] */
        unsigned int rsv_76 : 16;          /* [63:48] */
    } bits;

    unsigned int value;
} u_txdma_ar_cnt_mster0;

typedef union {
    struct {
        unsigned int txdma_ar_cnt_m1 : 48; /* [47:0] */
        unsigned int rsv_77 : 16;          /* [63:48] */
    } bits;

    unsigned int value;
} u_txdma_ar_cnt_mster1;

typedef union {
    struct {
        unsigned int txdma_r_cnt_m0 : 48; /* [47:0] */
        unsigned int rsv_78 : 16;         /* [63:48] */
    } bits;

    unsigned int value;
} u_txdma_r_cnt_mster0;

typedef union {
    struct {
        unsigned int txdma_r_cnt_m1 : 48; /* [47:0] */
        unsigned int rsv_79 : 16;         /* [63:48] */
    } bits;

    unsigned int value;
} u_txdma_r_cnt_mster1;

typedef union {
    struct {
        unsigned int txdma_axi_tx_out_cnt : 48; /* [47:0] */
        unsigned int rsv_80 : 16;               /* [63:48] */
    } bits;

    unsigned int value;
} u_txdma_axi_tx_out_cnt;

typedef union {
    struct {
        unsigned int rsv_81 : 4;                   /* [3:0] */
        unsigned int cfg_tpu2ssu_perf_test_en : 1; /* [4] */
        unsigned int rsv_82 : 3;                   /* [7:5] */
        unsigned int cfg_dmaport_perf_test_en : 1; /* [8] */
        unsigned int rsv_83 : 3;                   /* [11:9] */
        unsigned int cfg_bus_delay_test_en : 1;    /* [12] */
        unsigned int rsv_84 : 19;                  /* [31:13] */
    } bits;

    unsigned int value;
} u_tpu_perf_test_sel;

typedef union {
    struct {
        unsigned int rsv_85 : 8;                     /* [7:0] */
        unsigned int cfg_tpu2ssu_perf_test_port : 6; /* [13:8] */
        unsigned int rsv_86 : 2;                     /* [15:14] */
        unsigned int cfg_dmaport_perf_test_port : 6; /* [21:16] */
        unsigned int rsv_87 : 10;                    /* [31:22] */
    } bits;

    unsigned int value;
} u_tpu_perf_test_port;

typedef union {
    struct {
        unsigned int cfg_bus_delay_test_group_sel : 5; /* [4:0] */
        unsigned int rsv_88 : 27;                      /* [31:5] */
    } bits;

    unsigned int value;
} u_tpu_bus_delay_group_sel;

typedef union {
    struct {
        unsigned int tpu2ssu_perf_test_byte_status : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tpu2ssu_perf_test_byte_status;

typedef union {
    struct {
        unsigned int tpu2ssu_perf_test_cnt_status : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tpu2ssu_perf_test_cnt_status;

typedef union {
    struct {
        unsigned int dmaport_perf_cnt_status : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_dmaport_perf_test_cnt_status;

typedef union {
    struct {
        unsigned int tpu_bus_delay_cnt0 : 48; /* [47:0] */
        unsigned int rsv_89 : 16;             /* [63:48] */
    } bits;

    unsigned int value;
} u_tpu_bus_delay_cnt0;

typedef union {
    struct {
        unsigned int tpu_bus_delay_cnt1 : 48; /* [47:0] */
        unsigned int rsv_90 : 16;             /* [63:48] */
    } bits;

    unsigned int value;
} u_tpu_bus_delay_cnt1;

typedef union {
    struct {
        unsigned int tpu_bus_delay_cnt2 : 48; /* [47:0] */
        unsigned int rsv_91 : 16;             /* [63:48] */
    } bits;

    unsigned int value;
} u_tpu_bus_delay_cnt2;

typedef union {
    struct {
        unsigned int tpu_bus_delay_cnt3 : 48; /* [47:0] */
        unsigned int rsv_92 : 16;             /* [63:48] */
    } bits;

    unsigned int value;
} u_tpu_bus_delay_cnt3;

typedef union {
    struct {
        unsigned int tpu_bus_delay_cnt4 : 48; /* [47:0] */
        unsigned int rsv_93 : 16;             /* [63:48] */
    } bits;

    unsigned int value;
} u_tpu_bus_delay_cnt4;

typedef union {
    struct {
        unsigned int tpu_bus_delay_cnt5 : 48; /* [47:0] */
        unsigned int rsv_94 : 16;             /* [63:48] */
    } bits;

    unsigned int value;
} u_tpu_bus_delay_cnt5;

typedef union {
    struct {
        unsigned int tpu_bus_delay_cnt6 : 48; /* [47:0] */
        unsigned int rsv_95 : 16;             /* [63:48] */
    } bits;

    unsigned int value;
} u_tpu_bus_delay_cnt6;

typedef union {
    struct {
        unsigned int tpu_bus_delay_cnt7 : 48; /* [47:0] */
        unsigned int rsv_96 : 16;             /* [63:48] */
    } bits;

    unsigned int value;
} u_tpu_bus_delay_cnt7;

#endif  // __REG_TPU_H__
