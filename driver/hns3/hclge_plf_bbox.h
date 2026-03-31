/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Description: bbox
 * Create: 2023-02-28
 */

#ifndef __HCLGE_PLF_BBOX_H
#define __HCLGE_PLF_BBOX_H

#include "bbox_pub.h"

/* exception function id: 5-bit */
enum {
    HCLGE_PLF_BBOX_FUNC_TEST,
    HCLGE_PLF_BBOX_FUNC_NIC,
    HCLGE_PLF_BBOX_FUNC_DOT,

    HCLGE_PLF_BBOX_FUNC_MAX = 32
};

/* HCLGE_PLF_BBOX_FUNC_NIC sub_id: 12-bit */
enum {
    HCLGE_PLF_BBOX_NIC_UNINIT_NOTIFY,
    HCLGE_PLF_BBOX_NIC_GLOBAL_RESET,
    HCLGE_PLF_BBOX_NIC_PORT_RESET,

    HCLGE_PLF_BBOX_NIC_E_MAX = 4096
};

/* HCLGE_PLF_BBOX_FUNC_DOT sub_id: 12-bit */
enum {
    HCLGE_PLF_BBOX_DOT_SUSPEND,
    HCLGE_PLF_BBOX_DOT_RESUME,

    HCLGE_PLF_BBOX_DOT_E_MAX = 4096
};

/* suspend scene dot_id: 8-bit */
enum {
    HCLGE_PLF_DOT_SUSPEND_CLEAR_BIT,
    HCLGE_PLF_DOT_SUSPEND_BEGIN,
    HCLGE_PLF_DOT_SUSPEND_FUNC_RESET,
    HCLGE_PLF_DOT_SUSPEND_DIS_MAC_DOWN,
    HCLGE_PLF_DOT_SUSPEND_DIS_MAC_UP,
    HCLGE_PLF_DOT_SUSPEND_RELEASE_MAC_IRQ,

    HCLGE_PLF_DOT_SUSPEND_END = 255
};

/* resume scene dot_id: 8-bit */
enum {
    HCLGE_PLF_DOT_RESUME_CLEAR_BIT,
    HCLGE_PLF_DOT_RESUME_BEGIN,
    HCLGE_PLF_DOT_RESUME_RESET_DONE,
    HCLGE_PLF_DOT_RESUME_REQUEST_MAC_IRQ,

    HCLGE_PLF_DOT_RESUME_END = 255
};

/*
 * function id:5bit
 * level: 3 bit BBOX_OTHER/BBOX_NOTICE/BBOX_MINOR/BBOX_MAJOR/BBOX_CRITICAL
 * sub_id:12bit
 */
#define HCLGE_PLF_BBOX_MSG_DEVBIT_VAL         0x2
#define HCLGE_PLF_BBOX_MSG_DEVBIT_OFFSET      30
#define HCLGE_PLF_BBOX_MSG_EXPBIT_VAL         0x2
#define HCLGE_PLF_BBOX_MSG_EXPBIT_OFFSET      28
#define HCLGE_PLF_BBOX_MSG_ALARMBIT_OFFSET    25
#define HCLGE_PLF_BBOX_MSG_MODULEID_OFFSET    17
#define HCLGE_PLF_BBOX_MSG_MODULEFUNC_OFFSET  12
#define HCLGE_PLF_BBOX_EXCEP_TIME_INIT        6
#define HCLGE_PLF_BBOX_EXCEP_TIME_CUTOFF      1000

#define HCLGE_PLF_EXCEPID(func, level, sub_id) \
    ((((u32)HCLGE_PLF_BBOX_MSG_DEVBIT_VAL) << HCLGE_PLF_BBOX_MSG_DEVBIT_OFFSET) | \
     (((u32)HCLGE_PLF_BBOX_MSG_EXPBIT_VAL) << HCLGE_PLF_BBOX_MSG_EXPBIT_OFFSET) | \
     (((u32)level) << HCLGE_PLF_BBOX_MSG_ALARMBIT_OFFSET) | \
     (((u32)BBOX_NETWORK) << HCLGE_PLF_BBOX_MSG_MODULEID_OFFSET) | \
     (((u32)func) << HCLGE_PLF_BBOX_MSG_MODULEFUNC_OFFSET) | ((u32)sub_id))

/*
 * EXCEPTION ID DEFINES
 */
#define HCLGE_PLF_EXCEPID_NIC_UNINIT              HCLGE_PLF_EXCEPID(HCLGE_PLF_BBOX_FUNC_NIC, \
                                                                    BBOX_NOTICE, \
                                                                    HCLGE_PLF_BBOX_NIC_UNINIT_NOTIFY)
#define HCLGE_PLF_EXCEPID_NIC_GLOBAL_RESET        HCLGE_PLF_EXCEPID(HCLGE_PLF_BBOX_FUNC_NIC, \
                                                                    BBOX_NOTICE, \
                                                                    HCLGE_PLF_BBOX_NIC_GLOBAL_RESET)
#define HCLGE_PLF_EXCEPID_NIC_PORT_RESET          HCLGE_PLF_EXCEPID(HCLGE_PLF_BBOX_FUNC_NIC, \
                                                                    BBOX_NOTICE, \
                                                                    HCLGE_PLF_BBOX_NIC_PORT_RESET)

#define HCLGE_PLF_EXCEPID_DOT_SUSPEND             HCLGE_PLF_EXCEPID(HCLGE_PLF_BBOX_FUNC_DOT, \
                                                                    BBOX_CRITICAL, \
                                                                    HCLGE_PLF_BBOX_DOT_SUSPEND)
#define HCLGE_PLF_EXCEPID_DOT_RESUME              HCLGE_PLF_EXCEPID(HCLGE_PLF_BBOX_FUNC_DOT, \
                                                                    BBOX_CRITICAL, \
                                                                    HCLGE_PLF_BBOX_DOT_RESUME)

/* blockdot */
#define HCLGE_PLF_BLOCKDOT_DEV_NUM                         4U
#define HCLGE_PLF_BLOCKDOT_STATUS_SHIFT                    8U
#define HCLGE_PLF_BLOCKDOT_STATUS_OFFSET(dot_id, hdev_id)  ((dot_id) << ((hdev_id) * HCLGE_PLF_BLOCKDOT_STATUS_SHIFT))
#define HCLGE_PLF_BLOCKDOT_STATUS_CLEAR_DOT(hdev_id)       (~(0xFF << ((hdev_id) * HCLGE_PLF_BLOCKDOT_STATUS_SHIFT)))
#define HCLGE_PLF_BLOCKDOT_FLAG_DISABLE                    false
#define HCLGE_PLF_BLOCKDOT_FLAG_ENABLE                     true
#define HCLGE_PLF_BLOCKDOT_MAGIC_NUM                       0x9F3B4A7C
#define HCLGE_PLF_BLOCKDOT_BLOCK_ID                        13U

/* snapshot */
#define HCLGE_PLF_BBOX_LOG_MEM_ADDR                   0x23900000
#define HCLGE_PLF_BBOX_LOG_MEM_ADDR_MASK              0xFFFFFFFF
#define HCLGE_PLF_BBOX_LOG_RESV_MEM_LEN               0x10000             /* 64KB */
#define HCLGE_PLF_BBOX_BLOCK_NUM                      1
#define HCLGE_PLF_BBOX_NIC_BLOCK_IDX                  0
#define HCLGE_PLF_BBOX_DEV_NUM                        4U
#define HCLGE_PLF_BBOX_DDR_SIZE_PER_DEV               0x200000000000UL    /* for future use */
#define HCLGE_PLF_BBOX_MSLEEP_TIME                    300
#define HCLGE_PLF_BBOX_LOG_BUF_LEN                    0x80

typedef void (*hclge_plf_excep_handler)(const struct bbox_dump_ops_info *, const char *);
struct hclge_plf_bbox_excep_ops {
    bbox_exception_info_s info;
    hclge_plf_excep_handler handler;
    char log_buf[HCLGE_PLF_BBOX_LOG_BUF_LEN];
};

struct hclge_plf_bbox_cb {
    atomic_t report_flag;
    atomic_t report_cnt;
    struct bbox_module_result log_mem;
    char *va[HCLGE_PLF_BBOX_DEV_NUM];
};

#define HCLGE_PLF_BBOX_LOG_LEN_MAX    0x8000
struct hclge_plf_bbox_info {
    struct bbox_module_ctrl head;
    char log[HCLGE_PLF_BBOX_LOG_LEN_MAX];
};

struct hclge_plf_dot_cb {
    bool blockdot_flag;
    u32 blockdot_status;
    u32 blockdot_dev_cnt;
    struct mutex blockdot_mutex;
};

int hclge_plf_bbox_init(void);
void hclge_plf_bbox_uninit(void);
void hclge_plf_bbox_excep_report(u32 devid, u32 except_id);
void hclge_plf_blockdot_begin(u32 hdev_id, u32 id_map, u32 except_id, u32 dot_id);
void hclge_plf_blockdot_record(u32 hdev_id, u32 dot_id);
void hclge_plf_blockdot_end(u32 hdev_id, u32 dot_id);
#endif // __HCLGE_PLF_BBOX_H

