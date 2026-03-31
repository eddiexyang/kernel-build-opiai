/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2022-08-13
 */

#ifndef BBOX_COMMON_H
#define BBOX_COMMON_H
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>

/*
 * snapshot log for startup not exception
 */
#define BBOX_BOOT_STATUS_BLOCK_MAX_NUM  10
#define BOOT_DOT_ID             0xB2600000
#define DEVICE_NUM              0x1

#define PATH_MAXLEN             128
#define DATA_MAXLEN             15
#define DATATIME_MAXLEN         25U  /* 14+9+2, 2: '-'+'\0' */
#define NAME_MAXLEN             32

#ifdef DEBUG
#define WAIT_MS_VSHORT      10      /* very short */
#define WAIT_MS_SHORT       20     /* short */
#define WAIT_MS_MIDDLE      50
#define WAIT_MS_LONG        100
#else
#define WAIT_MS_VSHORT      50      /* very short */
#define WAIT_MS_SHORT       100     /* short */
#define WAIT_MS_MIDDLE      500
#define WAIT_MS_LONG        1000
#endif

#define BUF_LEN_BIG         1024
#define BUF_LEN_MID         512
#define BUF_LEN_SMALL       128

#define TWENTY_CENTURY  1900
#define JANUARY         1
#define KILO            1000U

#define BBOX_MALLOC_MAX_SIZE 0x8000000  // 128M

#ifndef UNUSED
#define UNUSED(x)   do {(void)(x);} while (0)
#endif

#define BBOX_MIN(a, b)          (((a) < (b)) ? (a) : (b))
#define BBOX_MAX(a, b)          (((a) > (b)) ? (a) : (b))

#define TS_NSEC2USEC(nsec)          ((nsec) / 1000)
#define TS_USEC2SEC_TRUNC(nsec)     ((nsec) / 1000000)
#define TS_USEC2SEC_REMAIN(nsec)    ((nsec) % 1000000)

#define TOLOWER(x)      (((x) >= 'a') ? (x) : (((x) - 'A') + 'a'))
#define FORMAT_HEAD_LEN 2
#define FORMAT_HEX      16U
#define FORMAT_DEC      10
#define STRTOL_LEN      16
#define BBOX_MEM_CPY_LEN 0x200000
#define BBOX_WAIT_MIN_TIME 1000
#define BBOX_WAIT_MAX_TIME 1500

#define BBOX_VFREE(buffer) do { \
    if ((buffer) != NULL) {     \
        vfree(buffer);          \
        (buffer) = NULL;        \
    }                           \
} while (0)

#define BBOX_KFREE(buffer) do { \
    if ((buffer) != NULL) {    \
        kfree(buffer);         \
        (buffer) = NULL;       \
    }                          \
} while (0)

#define KTHREAD_STOP(task) do { \
    if ((task) != NULL) {       \
        (void)kthread_stop(task);     \
        (task) = NULL;          \
    }                           \
} while (0)

struct bbox_time {
    u64 tv_sec;
    u64 tv_nsec;
};

#define ADAPTER_DUMP_MAGIC      0x19283746
#define ADAPTER_DUMP_END_MAGIC  0x1F2E3D4C
#define ADAPTER_VERSION         0x10002 // v1.0.2
#define AP_LOG_BUFFER_NUM       10U

// macro abort device
#define DEFAULT_DEVICE_ID   0UL
#define DEVICE_MAX_NUM      4U

#define LPM_SRAM_MAX_SIZE   0x10000 // 64KB
#define LPFW_SRAM_MAX_SIZE  0x10000 // 64KB
#define HDR_MAX_SIZE        0x80000 // 512KB
#define CDR_MAX_SIZE        0xA00000  // 10M

#define DUMP_LOG_MAX_TIME   120000

#define BBOX_AREA_MAXIMUM   32U
#define REGS_DUMP_MAX_NUM   10U
#define REG_NAME_LEN        12U
#define BBOX_BLOCK_MAX_NUM    128U
#define BBOX_BOOT_STATUS_VALUE_PAIR  2U


#define BBOX_RESERVED_ADDR_MAX 0x50000000
#define BBOX_RESERVED_SIZE_MAX 0x1E00000


enum excepid_ap {
    EXCEPID_AP_OOM            = 0xA4040001U,
    EXCEPID_AP_COMM           = 0xA6040001U,
    EXCEPID_AP_REBOOT         = 0xA8040000U,
    EXCEPID_AP_PANIC          = 0xA8040001U
};

enum excepid_hot_boot {
    EXCEPID_SUSPEND_FAIL       = 0xA6600001U,
    EXCEPID_RESUME_FAIL        = 0xA6600002U
};

typedef struct {
    char reg_name[REG_NAME_LEN];
    u32 reg_size;
    u64 reg_base;
    u64 reg_map_addr;    // reg virtual address
    u64 reg_dump_addr;   // dump virtual address
} regs_info;

struct ap_top_head {
    u32 dump_magic;
    u32 version;         // ap version
    u32 device_num;
    u32 cpu_num;
    u32 reserve;
    u32 end_magic;
};

struct ap_current_info {
    u32 devid;
    u32 excepid;
    u8  etype;
    u8  coreid;
    u16 reserve;
    struct bbox_time tm;
};

struct ap_log_record {
    u32 devid;
    u32 excepid;
    u8  etype;
    u8  coreid;
    u16 reserve;
    char date[DATATIME_MAXLEN];
};

struct ap_log_info {
    u16 event_flag;
    u8 log_num;
    u8 next_valid_index;
    struct ap_log_record log_buffer[AP_LOG_BUFFER_NUM];
};

struct ap_area_info {
    u64 ap_area_map_addr;   // reserved virtual address
    u64 ap_area_addr;       // reserved physical address
    u32 ap_area_len;        // reserved physical address length
    u32 num_reg_regions;
    regs_info dump_regs_info[DEVICE_MAX_NUM][REGS_DUMP_MAX_NUM];      // regs info for all deivce
    u32 num_reg_regions_core;                                         // regs info for aos core
    regs_info dump_regs_info_core[DEVICE_MAX_NUM][REGS_DUMP_MAX_NUM]; // regs info for aos core
};

struct ap_root {
    struct ap_top_head top_head;
    struct ap_current_info current_info;
    struct ap_log_info log_info;
    struct ap_area_info area_info;
    struct ap_top_head core_top_head;          // for aos core
    struct ap_current_info core_current_info;  // for aos core
    struct ap_log_info core_log_info;          // for aos core
};

static inline bool bbox_isdigit(char c)
{
    return ((c >= '0') && (c <= '9'));
}

static inline u32 bbox_double(u32 num)
{
    return (num * 2U);
}

typedef s32 (*pkthreadFunc)(void *arg);

void *bbox_kmalloc(size_t size);
void bbox_kfree(void *buffer);

void *bbox_vmalloc(size_t size);
void bbox_vfree(void *buffer);

void bbox_get_systime(struct bbox_time *tm);
void bbox_get_safe_systime(struct bbox_time *tm);
void bbox_get_sysdate(const struct bbox_time *tm, char *date, u32 len);
void bbox_get_date(const struct bbox_time *tm, char *date, u32 len);
u32 bbox_get_time_seq(void);
void bbox_update_time_seq(struct bbox_time *tm);

s32 bbox_strtouint_h(const char *s, u32 *val);
s32 bbox_strtol_h(const char *s, u64 *val);

void *bbox_ioremap(u64 paddr, u64 size);
void bbox_iounmap(const void *vaddr);
struct task_struct *bbox_kthread_proc(pkthreadFunc func, void *data, const char *threadname);

#if (defined BBOX_SOC_PLATFORM_MDC) && (!defined BBOX_UT)
s32 bbox_copy_exception_msg(void *dest, const char *src, u32 len);
int bbox_get_ctrlcpu_mask(void);
#endif
#ifdef AOS_LLVM_BUILD
u32 bbox_get_device_num(void);
#endif

#endif

