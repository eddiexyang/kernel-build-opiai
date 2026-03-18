/* SPDX-License-Identifier: GPL-2.0+ */
// Copyright (c) 2016-2017 Hisilicon Limited.

#ifndef __HNS3_BBOX_H
#define __HNS3_BBOX_H

#include "bbox_pub.h"
#include "hnae3.h"

/* HNS_BBOX_FUNC_IMP sub_id: 12-bit */
enum {
    IMP_TEST_NOTIFY,
    IMP_RESET,
    IMP_LOG_FULL,
    IMP_NMI_INTER,
    IMP_HARD_FAULT,
    IMP_MEM_FAULT,
    IMP_BUS_FAULT,
    IMP_USAGE_FAULT,
    IMP_SVCALL_FAULT,
    IMP_DEBUG_HANDLE,
    IMP_PENDSV,
    IMP_UNHANDLE_FAULT,
    IMP_PFC_STORM,
    IMP_PAUSE_STORM,

    HNS_BBOX_IMP_E_MAX = 4096
};

/* HNS_BBOX_FUNC_CUSTOM sub_id: 12-bit */
enum {
    HNS_BBOX_CUSTOM_INIT_NOTIFY,
    HNS_BBOX_CUSTOM_UNINIT_NOTIFY,

    HNS_BBOX_CUSTOM_E_MAX = 4096
};

/*
 * function id:5bit
 * level: 3 bit BBOX_OTHER/BBOX_NOTICE/BBOX_MINOR/BBOX_MAJOR/BBOX_CRITICAL
 * sub_id:12bit
 */
#define HNS_BBOX_MSG_DEVBIT_VAL         0x2
#define HNS_BBOX_MSG_DEVBIT_OFFSET      30
#define HNS_BBOX_MSG_EXPBIT_VAL         0x2
#define HNS_BBOX_MSG_EXPBIT_OFFSET      28
#define HNS_BBOX_MSG_ALARMBIT_OFFSET    25
#define HNS_BBOX_MSG_MODULEID_OFFSET    17
#define HNS_BBOX_MSG_MODULEFUNC_OFFSET  12
#define HNS_BBOX_EXCEP_TIME_INIT        6

#define HNS_EXCEPID(func, level, sub_id)    ((((u32)HNS_BBOX_MSG_DEVBIT_VAL) << HNS_BBOX_MSG_DEVBIT_OFFSET) | \
                        (((u32)HNS_BBOX_MSG_EXPBIT_VAL) << HNS_BBOX_MSG_EXPBIT_OFFSET) | \
                        (((u32)level) << HNS_BBOX_MSG_ALARMBIT_OFFSET) | \
                        (((u32)BBOX_NETWORK) << HNS_BBOX_MSG_MODULEID_OFFSET) | \
                        (((u32)func) << HNS_BBOX_MSG_MODULEFUNC_OFFSET) | ((u32)sub_id))

/*
 * HNS EXCEPTION ID DEFINES
 */
#define HNS_EXCEPID_NIC_INIT        HNS_EXCEPID(HNS_BBOX_FUNC_NIC, BBOX_NOTICE, HNS_BBOX_NIC_INIT_NOTIFY)
#define HNS_EXCEPID_NIC_UNINIT      HNS_EXCEPID(HNS_BBOX_FUNC_NIC, BBOX_NOTICE, HNS_BBOX_NIC_UNINIT_NOTIFY)
#define HNS_EXCEPID_NIC_RAS_ERR     HNS_EXCEPID(HNS_BBOX_FUNC_NIC, BBOX_MAJOR, HNS_BBOX_NIC_RAS_E)

#define HNS_EXCEPID_RoCE_INIT       HNS_EXCEPID(HNS_BBOX_FUNC_ROCE, BBOX_NOTICE, HNS_BBOX_ROCE_INIT_NOTIFY)
#define HNS_EXCEPID_RoCE_UNINIT     HNS_EXCEPID(HNS_BBOX_FUNC_ROCE, BBOX_NOTICE, HNS_BBOX_ROCE_UNINIT_NOTIFY)
#define HNS_EXCEPID_ABNM_CQ_CAUSE_CQE_ERR  HNS_EXCEPID(HNS_BBOX_FUNC_ROCE, BBOX_MINOR, HNS_BBOX_ROCE_WQE_COTNENT_E)
#define HNS_EXCEPID_SQ_DRAINED_ERR         HNS_EXCEPID(HNS_BBOX_FUNC_ROCE, BBOX_MINOR, HNS_BBOX_ROCE_SQ_DRAINED_E)
#define HNS_EXCEPID_INV_REQ_LOCAL_WQ_ERR   HNS_EXCEPID(HNS_BBOX_FUNC_ROCE, BBOX_MINOR, HNS_BBOX_ROCE_REQ_INVALID_OPCODE_E)
#define HNS_EXCEPID_LOC_WQ_ACCESS_ERR      HNS_EXCEPID(HNS_BBOX_FUNC_ROCE, BBOX_MINOR, HNS_BBOX_ROCE_LOC_WQ_ACCESS_E)
#define HNS_EXCEPID_SRQ_CATAS_ERR          HNS_EXCEPID(HNS_BBOX_FUNC_ROCE, BBOX_MINOR, HNS_BBOX_ROCE_SRQ_CATAS_E)
#define HNS_EXCEPID_CQ_OVERFLOW_ERR        HNS_EXCEPID(HNS_BBOX_FUNC_ROCE, BBOX_MINOR, HNS_BBOX_ROCE_CQ_OVERFLOW_E)
#define HNS_EXCEPID_DB_OVERFLOW_ERR        HNS_EXCEPID(HNS_BBOX_FUNC_ROCE, BBOX_MINOR, HNS_BBOX_ROCE_DB_OVERFLOW_E)

#define HNS_EXCEPID_IMP_TEST_NOTIFY        HNS_EXCEPID(HNS_BBOX_FUNC_IMP, BBOX_NOTICE, IMP_TEST_NOTIFY)
#define HNS_EXCEPID_IMP_RESET              HNS_EXCEPID(HNS_BBOX_FUNC_IMP, BBOX_NOTICE, IMP_RESET)
#define HNS_EXCEPID_IMP_LOG_FULL           HNS_EXCEPID(HNS_BBOX_FUNC_IMP, BBOX_MINOR, IMP_LOG_FULL)
#define HNS_EXCEPID_IMP_NMI_INTER          HNS_EXCEPID(HNS_BBOX_FUNC_IMP, BBOX_MAJOR, IMP_NMI_INTER)
#define HNS_EXCEPID_IMP_HARD_FAULT         HNS_EXCEPID(HNS_BBOX_FUNC_IMP, BBOX_MAJOR, IMP_HARD_FAULT)
#define HNS_EXCEPID_IMP_MEM_FAULT          HNS_EXCEPID(HNS_BBOX_FUNC_IMP, BBOX_MAJOR, IMP_MEM_FAULT)
#define HNS_EXCEPID_IMP_BUS_FAULT          HNS_EXCEPID(HNS_BBOX_FUNC_IMP, BBOX_MAJOR, IMP_BUS_FAULT)
#define HNS_EXCEPID_IMP_USAGE_FAULT        HNS_EXCEPID(HNS_BBOX_FUNC_IMP, BBOX_MAJOR, IMP_USAGE_FAULT)
#define HNS_EXCEPID_IMP_SVCALL_FAULT       HNS_EXCEPID(HNS_BBOX_FUNC_IMP, BBOX_MAJOR, IMP_SVCALL_FAULT)
#define HNS_EXCEPID_IMP_DEBUG_HANDLE       HNS_EXCEPID(HNS_BBOX_FUNC_IMP, BBOX_MAJOR, IMP_DEBUG_HANDLE)
#define HNS_EXCEPID_IMP_PENDSV             HNS_EXCEPID(HNS_BBOX_FUNC_IMP, BBOX_MAJOR, IMP_PENDSV)
#define HNS_EXCEPID_IMP_UNHANDLE_FAULT     HNS_EXCEPID(HNS_BBOX_FUNC_IMP, BBOX_MAJOR, IMP_UNHANDLE_FAULT)
#define HNS_EXCEPID_IMP_PFC_STORM          HNS_EXCEPID(HNS_BBOX_FUNC_IMP, BBOX_MINOR, IMP_PFC_STORM)
#define HNS_EXCEPID_IMP_PAUSE_STORM        HNS_EXCEPID(HNS_BBOX_FUNC_IMP, BBOX_MINOR, IMP_PAUSE_STORM)

#define HNS_EXCEPID_CUSTOM_INIT        HNS_EXCEPID(HNS_BBOX_FUNC_CUSTOM, BBOX_NOTICE, HNS_BBOX_CUSTOM_INIT_NOTIFY)
#define HNS_EXCEPID_CUSTOM_UNINIT      HNS_EXCEPID(HNS_BBOX_FUNC_CUSTOM, BBOX_NOTICE, HNS_BBOX_CUSTOM_UNINIT_NOTIFY)

#define HNS_BBOX_LOG_MEM_ADDR                   0x36520000
#define HNS_BBOX_LOG_MEM_ADDR_MASK              0xFFFFFFFF
#define HNS_BBOX_LOG_RESV_MEM_LEN               0x40000
#define HNS_BBOX_BLOCK_NUM                      3
#define HNS_BBOX_NIC_ROCE_BLOCK_IDX             0
#define HNS_BBOX_IMP_BLOCK_IDX                  1
#define HNS_BBOX_RAS_BLOCK_IDX                  2
#define HNS_BBOX_DEV_NUM                        4U
#define HNS_BBOX_CHIP_NUM                       16U
#define HNS_BBOX_MSLEEP_TIME                    300
#define HNS_BBOX_LOG_BUF_LEN                    0x80
#define HNS_BBOX_EXCEP_TIME_CUTOFF              1000

typedef void (*hns_excep_handler)(const struct bbox_dump_ops_info *);
typedef int (*hns_excep_dump)(u32, u32, const char *);
struct hns_bbox_excep_ops {
    bbox_exception_info_s info;
    hns_excep_handler handler;
    hns_excep_dump dump_log_buf;
    char log_buf[HNS_BBOX_LOG_BUF_LEN];
    bool is_fault_report;
};

#define BBOX_HCCS_PHY_CHIP_BASE_ADDR            0x2000
#define BBOX_HCCS_PHY_CHIP_ADDR_OFFSET          0x200
#define BBOX_PHY_CHIP_ADDR_OFFSET               0x800
#define BBOX_CHIP_DIE_ADDR_OFFSET               0x100
#define BBOX_DIE_HIGH_OFFSET                    0x10

/* die0 do not need to add the high offset */
#define BBOX_ADDR_HIGH_OFFSET(chip_id, die_id, connect_type) \
    (connect_type == HCCS_CONNECT_TYPE ? \
     (die_id != 0x0 ? (BBOX_HCCS_PHY_CHIP_BASE_ADDR + (BBOX_HCCS_PHY_CHIP_ADDR_OFFSET * (chip_id)) + \
      (BBOX_CHIP_DIE_ADDR_OFFSET * (die_id)) + BBOX_DIE_HIGH_OFFSET) : 0x0) : \
     (die_id != 0x0 ? ((BBOX_PHY_CHIP_ADDR_OFFSET * (chip_id)) + \
      (BBOX_CHIP_DIE_ADDR_OFFSET * (die_id)) + BBOX_DIE_HIGH_OFFSET) : 0x0))

struct hns_bbox_cb {
    u32 chip_id;
    atomic_t report_flag;
    atomic_t report_cnt;
    struct bbox_module_result log_mem;
    u64 hns_imp_share_mem_addr;
    u32 hns_imp_share_mem_len;
    char *va_bbox[HNS_BBOX_DEV_NUM];
    char *va_imp[HNS_BBOX_DEV_NUM];
};

#define HNS_BBOX_LOG_LEN_MAX                    0x8000
#define HNS_BBOX_LOG_MSG_MAX                    0x100

struct hns_bbox_info {
    struct bbox_module_ctrl head;
    char log[HNS_BBOX_LOG_LEN_MAX];
    u32 pi;
    u32 ci;
};

#define HNS_BBOX_RAS_ADDR_OFFSET                0x10000
#define HNS_BBOX_RAS_BUF_LEN                    0x200
#define HNS_BBOX_RAS_MSG_MAX                    64

struct hns_bbox_ras_info {
    u32 pi;
    u32 ci;
};

#define HNS_BBOX_IMP_MSG_MAX                    16
#define HNS_BBOX_IMP_ADDR_OFFSET                0x20000
#define HNS_BBOX_IMP_LOG_LEN_MAX                0x2000
#define HNS_IMP_SHARE_MEM_ADDR                  0x31F00000
#define HNS_IMP_SHARE_MEM_LEN                   0x100000
#define HNS_IMP_SHARE_BUF_MAGIC                 0xBEEFA55A

struct hns_bbox_share_mem {
    u32 except_id;
    u32 offset;
    u32 len;
    u32 resv;
};

struct hns_bbox_imp_info {
    u32 magic;
    u32 pi;
    u32 ci;
    u32 rsvd;
    struct hns_bbox_share_mem mem[HNS_BBOX_IMP_MSG_MAX];
};

struct hns_imp_bbox_msg {
    u32 except_id;
    u32 rsvd;
    u64 sec;
    u64 usec;
};

struct hns_imp_share_mem {
    u32 magic;
    u32 pi;
    u32 ci;
    u32 rsvd;
    struct hns_imp_bbox_msg msg[HNS_BBOX_IMP_MSG_MAX];
};

struct hns_bbox_cb *hns_bbox_get_cb(void);
int hns_bbox_init(u32 chip_id);
void hns_bbox_uninit(void);
int hns_get_bbox_excepid(u32 func_id, u32 sub_id, u32 *except_id);
void hns_bbox_excep_report(u32 devid, u32 except_id);
void hns_imp_bbox_report(u32 devid);
void hns_bbox_ras_excep_report(u32 devid, const char *format, ...);
bool hns_is_addr_in_white_list(u64 addr);
bool hns_imp_share_mem_is_valid(u64 addr, u32 len);
bool hns_bbox_log_mem_is_valid(void);
#endif
