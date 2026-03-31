/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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
 * Create: 2019-10-15
 */
#ifndef PROF_DRV_DEV_H
#define PROF_DRV_DEV_H

#include "prof.h"
#include "prof_def.h"
#include "prof_peripheral.h"
#include "prof_ts.h"

#define MODULE_PROF "drv_prof"
#ifdef AOS_LLVM_BUILD
#define PROF__GFP_THISNODE 0
#define PROF_GFP_HIGHUSER_MOVABLE 0
#else
#define PROF__GFP_THISNODE __GFP_THISNODE
#define PROF_GFP_HIGHUSER_MOVABLE GFP_HIGHUSER_MOVABLE
#endif
/*
 * profiling channel id:
 * 10 to (CHANNEL_TSCPU_MAX-1) are TSCPU type,
 * 1 to 9 and 129 to (CHANNEL_IDS_MAX-1) are Peripheral(CtrlCPU) type
 * >>>>>>>>>>>>>> should be Consistent with ascend_hal.h in infer310 <<<<<<<<<<<<
 * >>>>>>>>>>>>>> should be Consistent with ascend_hal.h in trunk_ai <<<<<<<<<<<<
 */
#define CHANNEL_HBM (1)
#define CHANNEL_BUS (2)
#define CHANNEL_PCIE (3)
#define CHANNEL_NIC (4)
#define CHANNEL_DMA (5)
#define CHANNEL_DVPP (6)
#define CHANNEL_DDR (7)
#define CHANNEL_LLC (8)
#define CHANNEL_HCCS (9)
#define CHANNEL_TSCPU (10)

#define CHANNEL_BIU_GROUP0_AIC (11)
#define CHANNEL_BIU_GROUP0_AIV0 (12)
#define CHANNEL_BIU_GROUP0_AIV1 (13)
#define CHANNEL_BIU_GROUP1_AIC (14)
#define CHANNEL_BIU_GROUP1_AIV0 (15)
#define CHANNEL_BIU_GROUP1_AIV1 (16)
#define CHANNEL_BIU_GROUP2_AIC (17)
#define CHANNEL_BIU_GROUP2_AIV0 (18)
#define CHANNEL_BIU_GROUP2_AIV1 (19)
#define CHANNEL_BIU_GROUP3_AIC (20)
#define CHANNEL_BIU_GROUP3_AIV0 (21)
#define CHANNEL_BIU_GROUP3_AIV1 (22)
#define CHANNEL_BIU_GROUP4_AIC (23)
#define CHANNEL_BIU_GROUP4_AIV0 (24)
#define CHANNEL_BIU_GROUP4_AIV1 (25)
#define CHANNEL_BIU_GROUP5_AIC (26)
#define CHANNEL_BIU_GROUP5_AIV0 (27)
#define CHANNEL_BIU_GROUP5_AIV1 (28)
#define CHANNEL_BIU_GROUP6_AIC (29)
#define CHANNEL_BIU_GROUP6_AIV0 (30)
#define CHANNEL_BIU_GROUP6_AIV1 (31)
#define CHANNEL_BIU_GROUP7_AIC (32)
#define CHANNEL_BIU_GROUP7_AIV0 (33)
#define CHANNEL_BIU_GROUP7_AIV1 (34)
#define CHANNEL_BIU_GROUP8_AIC (35)
#define CHANNEL_BIU_GROUP8_AIV0 (36)
#define CHANNEL_BIU_GROUP8_AIV1 (37)
#define CHANNEL_BIU_GROUP9_AIC (38)
#define CHANNEL_BIU_GROUP9_AIV0 (39)
#define CHANNEL_BIU_GROUP9_AIV1 (40)
#define CHANNEL_BIU_GROUP10_AIC (41)
#define CHANNEL_BIU_GROUP10_AIV0 (42)

#define CHANNEL_AICORE (43)
#define CHANNEL_TSFW (44)      // add for ts0 as tsfw channel
#define CHANNEL_HWTS_LOG (45)  // add for ts0 as hwts channel
#define CHANNEL_KEY_POINT (46)
#define CHANNEL_TSFW_L2 (47)   /* add for ascend910 and ascend610 */
#define CHANNEL_HWTS_LOG1 (48) // add for ts1 as hwts channel
#define CHANNEL_TSFW1 (49)     // add for ts1 as tsfw channel
#define CHANNEL_STARS_SOC_LOG_BUFFER (50)       /* add for ascend910B */
#define CHANNEL_STARS_BLOCK_LOG_BUFFER (51)     /* add for ascend910B */
#define CHANNEL_STARS_SOC_PROFILE_BUFFER (52)   /* add for ascend910B */
#define CHANNEL_FFTS_PROFILE_BUFFER_TASK (53)   /* add for ascend910B */
#define CHANNEL_FFTS_PROFILE_BUFFER_SAMPLE (54) /* add for ascend910B */

#define CHANNEL_BIU_GROUP10_AIV1 (55)
#define CHANNEL_BIU_GROUP11_AIC (56)
#define CHANNEL_BIU_GROUP11_AIV0 (57)
#define CHANNEL_BIU_GROUP11_AIV1 (58)
#define CHANNEL_BIU_GROUP12_AIC (59)
#define CHANNEL_BIU_GROUP12_AIV0 (60)
#define CHANNEL_BIU_GROUP12_AIV1 (61)
#define CHANNEL_BIU_GROUP13_AIC (62)
#define CHANNEL_BIU_GROUP13_AIV0 (63)
#define CHANNEL_BIU_GROUP13_AIV1 (64)
#define CHANNEL_BIU_GROUP14_AIC (65)
#define CHANNEL_BIU_GROUP14_AIV0 (66)
#define CHANNEL_BIU_GROUP14_AIV1 (67)
#define CHANNEL_BIU_GROUP15_AIC (68)
#define CHANNEL_BIU_GROUP15_AIV0 (69)
#define CHANNEL_BIU_GROUP15_AIV1 (70)
#define CHANNEL_BIU_GROUP16_AIC (71)
#define CHANNEL_BIU_GROUP16_AIV0 (72)
#define CHANNEL_BIU_GROUP16_AIV1 (73)
#define CHANNEL_BIU_GROUP17_AIC (74)
#define CHANNEL_BIU_GROUP17_AIV0 (75)
#define CHANNEL_BIU_GROUP17_AIV1 (76)
#define CHANNEL_BIU_GROUP18_AIC (77)
#define CHANNEL_BIU_GROUP18_AIV0 (78)
#define CHANNEL_BIU_GROUP18_AIV1 (79)
#define CHANNEL_BIU_GROUP19_AIC (80)
#define CHANNEL_BIU_GROUP19_AIV0 (81)
#define CHANNEL_BIU_GROUP19_AIV1 (82)
#define CHANNEL_BIU_GROUP20_AIC (83)
#define CHANNEL_BIU_GROUP20_AIV0 (84)

#define CHANNEL_AIV (85)

#define CHANNEL_BIU_GROUP20_AIV1 (86)
#define CHANNEL_BIU_GROUP21_AIC (87)
#define CHANNEL_BIU_GROUP21_AIV0 (88)
#define CHANNEL_BIU_GROUP21_AIV1 (89)
#define CHANNEL_BIU_GROUP22_AIC (90)
#define CHANNEL_BIU_GROUP22_AIV0 (91)
#define CHANNEL_BIU_GROUP22_AIV1 (92)
#define CHANNEL_BIU_GROUP23_AIC (93)
#define CHANNEL_BIU_GROUP23_AIV0 (94)
#define CHANNEL_BIU_GROUP23_AIV1 (95)
#define CHANNEL_BIU_GROUP24_AIC (96)
#define CHANNEL_BIU_GROUP24_AIV0 (97)
#define CHANNEL_BIU_GROUP24_AIV1 (98)

#define CHANNEL_TSCPU_MAX (128)
#define CHANNEL_ROCE (129)
#define CHANNEL_NPU_APP_MEM (130) /* HBM and DDR used on app level */
#define CHANNEL_NPU_MEM (131)     /* HBM and DDR used on device level */
#define CHANNEL_LP        (132)
#define CHANNEL_DVPP_VENC (135)  /* add for ascend610 */
#define CHANNEL_DVPP_JPEGE (136) /* add for ascend610 */
#define CHANNEL_DVPP_VDEC (137)  /* add for ascend610 */
#define CHANNEL_DVPP_JPEGD (138) /* add for ascend610 */
#define CHANNEL_DVPP_VPC (139)   /* add for ascend610 */
#define CHANNEL_DVPP_PNG (140)   /* add for ascend610 */
#define CHANNEL_DVPP_SCD (141)   /* add for ascend610 */
#define CHANNEL_IDS_MAX PROF_CHANNEL_NUM

/*
 * >>>>>>>>>>>>>> should be Consistent with ascend_hal.h in infer310 <<<<<<<<<<<<
 * >>>>>>>>>>>>>> should be Consistent with ascend_hal.h in trunk_ai <<<<<<<<<<<<
 */
enum prof_channel_type {
    PROF_TS_TYPE,
    PROF_PERIPHERAL_TYPE,
    PROF_CHANNEL_TYPE_MAX
};
#ifndef AOS_LLVM_BUILD
#define CHAR_DRIVER_NAME "prof_drv"
#else
#define PROF_NONE_ROOT_ACCESS 0600
#define CHAR_DRIVER_NAME "/dev/prof_drv"
#endif

#define DRV_MODE_VERSION "DAVINCI 1.01"

#define USR2DRV_TIMEOUT (3 * (HZ))

#define PROF_WAIT_NOTHING 0

#define PROF_TIMES_MS_TO_NS 1000000UL

#define PROF_CHECK_COUNT_MAX 3
#define PROF_WAIT_TS_COUNTS 50

#define PROF_DELAY_MS 200

#define DC_TSNUM  1
#ifdef CFG_SOC_MDC_V51_LITE
#define MDC_TSNUM 1
#else
#define MDC_TSNUM 2
#endif
/* real time type */
#define PROF_REAL     1

#define PROF_MODE_KERNEL 0
#define PROF_MODE_USER 1

#define PROF_VALID_TSCPU_FLAG       (int)0xABDDDD
#define PROF_VALID_PERIPHERAL_FLAG       (int)0xABDEDE
/*
 * 610 and 910 support computing virtualization:
 * 610 hardware and software channels support data split by vf,
 * 910 software channels support data split by vf;
 * HW means hardware,
 * SW means software;
 */
#define PROF_VALID_TSCPU_HW_VIRTUAL_FLAG    (int)0xABEFEE
#define PROF_VALID_TSCPU_SW_VIRTUAL_FLAG    (int)0xABEFFF
#define PROF_VALID_PERIPHERAL_VIRTUAL_FLAG  (int)0xABCDEF

#define PROF_PHYSICAL_MACHINE_VFID 0

#define PROF_ON_DEVICE 0
#define PROF_ON_HOST   1

#define BUFF_HEAD_SIZE 0x400
#define CACHE_LINE_LEN 128
#define PROF_PERIPHERAL_BUFFER_LEN (100 * 1024)
#define PROF_TS_BUFFER_LEN (1024 * 1024)
#define PROF_TS_BUFFER_LEN_MAX (4 * 1024 * 1024)
#define PROF_SAMPLE_PROC_NAME "prof_sample"

#ifndef PROF_OK
#define PROF_OK 0
#endif
#define PROF_ERROR (-1)
#define PROF_TIMEOUT (-2)
#define PROF_STARTED_ALREADY (-3)
#define PROF_STOPPED_ALREADY (-4)
#define PROF_ERESTARTSYS (-5)
#define PROF_NOT_READABLE_DATA (-6)
#define PROF_NOT_SUPPORT (-7)
#define PROF_BUSY (-8)
#define PROF_NOT_ENOUGH_BUF (-9)
#define PROF_NOT_ENOUGH_SUB_CHANNEL_RESOURCE (-10)
#define PROF_VF_SUB_RESOURCE_FULL (-11)
#define PROF_CONTAINER_SCENE_NOT_OPEN_PHY (-12)
#define PROF_PHY_SCENE_NOT_OPEN_CONTAINER (-13)

#define SAMPLE_MASK 0x01
#define SAMPLE_ONLY_DATA 0x0
#define SAMPLE_WITH_HEADER 0x1

#define PROF_CQSQ_INFO_SHOW(tsid, cqsq_info)  \
    prof_debug("[PROF_CQSQ_INFO]:tsid=%u, sq_0_index=%u, cq_0_index=%u, cq_1_index=%u", \
        (tsid), (cqsq_info)->sq_0_index, (cqsq_info)->cq_0_index, (cqsq_info)->cq_1_index)
#define PROF_SQ_CMD_SHOW(tsid, sq)  \
    prof_debug("[PROF_SQ_CMD]:tsid=%u, cmd_verify=%u, channel_id=%u, channel_cmd=%u, " \
        "buf_len=0x%x, phy_addr=%pK, buff_num=%u, vfid=%u, com_buf_phy_addr=%pK, sub_channel_id=%u, data_size=%u", \
        (tsid), (sq)->cmd_verify, (sq)->channel_id, (sq)->channel_cmd, (sq)->buf_len, \
        (void *)(uintptr_t)(sq)->buf_phy_addr, (sq)->buf_num, (sq)->vfid, (void *)(uintptr_t)(sq)->com_buf_phy_addr, \
        (sq)->sub_channel_id, (sq)->data_size)
#define PROF_HEAD_INFO_SHOW(a)  \
    prof_debug("[PROF_HEAD_INFO]:read_ptr=%u, writer_ptr=%u, buf_len = 0x%x.\n", \
        (a)->read_ptr, (a)->write_ptr, (a)->buf_len)
#define PROF_CQ_INFO_SHOW(cq_scheduler)  \
    prof_debug("[PROF_CQ_INFO] cmd_verify=%u, device_id=%u, channel_id=%u, channel_cmd=%u, ret_val=%d, " \
        "vfid=%u.\n", \
        (cq_scheduler)->cmd_verify, (cq_scheduler)->device_id, (cq_scheduler)->channel_id, \
        (cq_scheduler)->channel_cmd, (cq_scheduler)->ret_val, (cq_scheduler)->vfid)
#define PROF_CHANNEL_INFO_SHOW(channel_info)  \
    prof_debug("[PROF_CHANNEL_INFO] cmd_verify=%d, device_id=%d,  vfid=%d, channel_id=%d, channel_state=%d; " \
        "phy_addr = %pK, buf_len=0x%x.\n", \
        (channel_info)->ts_channel.cmd_verify, (channel_info)->device_id, (channel_info)->vfid, \
        (channel_info)->channel_id, (channel_info)->channel_state, (void *)(uintptr_t)(channel_info)->phy_addr, \
        (channel_info)->buf_len)

/* add for get prof channel list */
#define PROF_CHANNEL_NAME_LEN 32
struct channel_info {
    char channel_name[PROF_CHANNEL_NAME_LEN];
    u32 channel_type; /* system / APP */
    u32 channel_id;
};

typedef struct channel_list {
    u32 chip_type;
    u32 channel_num;
    struct channel_info channel[CHANNEL_IDS_MAX];
}channel_list_t;

/*
  * DEV_USED=ALL TS are ready
  * DEV_UNUSED=One of TS fail to init
  */
#define DEV_UNUSED 0
#define DEV_USED 1

#define TIME_UNIT (HZ)
#define PROF_TASK_SCHE_TIME_MS 20

enum prof_channel_state {
    PROF_CHANNEL_UNINIT,
    PROF_CHANNEL_IDLE,
    PROF_CHANNEL_DISABLE,
    PROF_CHANNEL_STARTING,
    PROF_CHANNEL_ENABLE,
    PROF_CHANNEL_FLUSH,
    PROF_CHANNEL_STOP_WAIT_TS,
    PROF_CHANNEL_STOPPING
};

// this Macro is defined to do unit test. Normal process will not involve
#ifdef PROF_UNIT_TEST
struct devdrv_info {
    int dev_id;
};
#define STATIC
static inline u32 drv_get_cur_processor_id(void)
{
    return 0;
}
#define prof_err(fmt, ...) ((void)printf(fmt, ##__VA_ARGS__))
#define prof_warn(fmt, ...) ((void)printf(fmt, ##__VA_ARGS__))
#define prof_info(fmt, ...) ((void)printf(fmt, ##__VA_ARGS__))
#define prof_debug(fmt, ...)
#define prof_event(fmt, ...) ((void)printf(fmt, ##__VA_ARGS__))
#define DEVDRV_MAX_TS_NUM 2
#else
#include "drv_smp_os_adapt.h"
#include "devdrv_manager_common.h"
#define STATIC static
#define prof_err(fmt, ...) \
    drv_err(MODULE_PROF, "<%s:%d,%d,%u> " fmt, current->comm, current->tgid, current->pid, \
        drv_get_cur_processor_id(), ##__VA_ARGS__)
#define prof_warn(fmt, ...) \
    drv_warn(MODULE_PROF, "<%s:%d,%d,%u> " fmt, current->comm, current->tgid, current->pid, \
        drv_get_cur_processor_id(), ##__VA_ARGS__)
#define prof_info(fmt, ...) \
    drv_info(MODULE_PROF, "<%s:%d,%d,%u> " fmt, current->comm, current->tgid, current->pid, \
        drv_get_cur_processor_id(), ##__VA_ARGS__)
#define prof_debug(fmt, ...) \
    drv_pr_debug(MODULE_PROF, "<%s:%d,%d,%u> " fmt, current->comm, current->tgid, current->pid, \
        drv_get_cur_processor_id(), ##__VA_ARGS__)
#define prof_event(fmt, ...) \
    drv_event(MODULE_PROF, "<%s:%d,%d,%u> " fmt, current->comm, current->tgid, current->pid, \
        drv_get_cur_processor_id(), ##__VA_ARGS__)

#define prof_err_spinlock(fmt, ...) \
    drv_err_spinlock(MODULE_PROF, "<%s:%d,%d,%u> " fmt, current->comm, current->tgid, current->pid, \
        drv_get_cur_processor_id(), ##__VA_ARGS__)
#define prof_warn_spinlock(fmt, ...) \
    drv_warn_spinlock(MODULE_PROF, "<%s:%d,%d,%u> " fmt, current->comm, current->tgid, current->pid, \
        drv_get_cur_processor_id(), ##__VA_ARGS__)
#define prof_info_spinlock(fmt, ...) \
    drv_info_spinlock(MODULE_PROF, "<%s:%d,%d,%u> " fmt, current->comm, current->tgid, current->pid, \
        drv_get_cur_processor_id(), ##__VA_ARGS__)
#define prof_debug_spinlock(fmt, ...)
#define prof_event_spinlock(fmt, ...) \
    drv_event_spinlock(MODULE_PROF, "<%s:%d,%d,%u> " fmt, current->comm, current->tgid, current->pid, \
        drv_get_cur_processor_id(), ##__VA_ARGS__)
#endif

enum prof_channel_mem_type {
    MEM_MALLOC,
    MEM_RESERVE,
    MEM_TYPE_MAX
};

/* size=1024 byte */
typedef struct prof_data_head {
    volatile u32 read_ptr;    /* ascd910 com_buff no care */
    volatile u32 buf_len;
    volatile u32 rev1[14];
    volatile u32 write_ptr;   /* ascd910 com_buff no care */
    volatile u32 rev2[239];
} prof_data_head_t;

typedef struct prof_vf_info {
    prof_channel_info_t channel_info[PROF_CHANNEL_NUM];
} prof_vf_info_t;

typedef struct prof_device_info {
    u32 device_id;
    u32 ts_num;
    u32 device_state;
    struct prof_cqsq_info cqsq_info[DEVDRV_MAX_TS_NUM];
    struct prof_vf_info vf_info[PROF_VFID_NUM_MAX];
} prof_device_info_t;

struct prof_channel_resource_info {
    struct mutex mutex;
    struct prof_sub_channel_info sub_resource_info[PROF_SUB_CHANNEL_NUM_MAX];
};

struct prof_device_resource_info {
    struct mutex mutex;
    struct prof_channel_resource_info channel_sub_resource_info[CHANNEL_IDS_MAX];
};


int prof_poll_init(struct prof_poll_info_kernel *prof_poll_mgr);
void prof_poll_free(struct prof_poll_info_kernel *prof_poll_mgr);
int prof_ctx_sub_channel_res_init(struct prof_sub_channel_res *sub_channel_res);
void prof_ctx_sub_channel_res_uninit(struct prof_sub_channel_res *sub_channel_res);

int prof_get_channel_list(struct prof_ioctl_para *para);
int prof_drv_stop(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *prof_para);
int prof_drv_read(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *prof_para);
int prof_poll(struct prof_proc_ctx *proc_ctx, struct prof_ioctl_para *para);

extern struct devdrv_info *devdrv_manager_get_devdrv_info(u32 device_id);
extern int devdrv_get_platformInfo(u32 *info);

bool prof_is_valid_channel(int channel_id);
int* prof_get_hash(void);
u32 prof_get_mem_type(int channel_id);
u64 prof_get_reserved_mem(u32 device_id, u32 channel_id, u32 sub_ch_id);

int prof_set_irq_affinity(unsigned int irq, const struct cpumask *cpumask);
int prof_clear_irq_affinity(unsigned int irq);
unsigned int prof_get_affinity_cpuid(u32 dev_id);
int prof_init_affinity_cpuid_thread(u32 device_id, struct task_struct *thread);
void prof_hdc_session_channel_free(u32 device_id, u32 vfid, u32 channel_id,
    struct prof_proc_ctx *proc_ctx);
extern bool tsdrv_is_ts_work(u32 devid, u32 tsid);

struct prof_device_info *prof_device_info_get(u32 device_id);
struct prof_channel_info *prof_get_channel_info_from_index(u32 device_id, u32 vfid,
    u32 chann_id);
enum prof_channel_type prof_get_channel_type(u32 channel_id);
int prof_wake_up_poll_wq(struct prof_sub_channel_info *sub_channel_info);
void prof_channel_info_free(struct prof_sub_channel_info *sub_channel_info, enum prof_channel_type channel_type);
int prof_send_data(void *outbuf, const void *buf, u32 size, struct prof_ioctl_para *para);
int prof_wait_read_last_data(struct prof_sub_channel_info *sub_channel_info);
u32 prof_tscpu_get_data_len(struct prof_sub_channel_info *sub_channel_info);
int prof_data_flush_support_check(u32 channel_id);

int prof_tscpu_send_sync_rw_ptr_cmd(struct prof_sub_channel_info *sub_channel_info, int refresh_type);
int prof_stop_channel_status_check(struct prof_sub_channel_info *sub_channel_info,
    struct prof_proc_ctx *proc_ctx);
bool prof_is_valid_virtual_tscpu_hw_channel(int channel_id);
bool prof_is_valid_virtual_tscpu_channel(int channel_id);
bool prof_is_valid_virtual_peri_channel(int channel_id);
int prof_check_device_state(u32 device_id);
struct prof_sub_channel_info *prof_alloc_sub_channel_resource(struct prof_ioctl_para *para,
    struct prof_proc_ctx *proc_ctx);
void prof_free_sub_channel_resource(struct prof_sub_channel_info *sub_channel_info);
int prof_get_available_sub_channel(struct prof_ioctl_para *para, struct prof_proc_ctx *proc_ctx,
    u32 phy_device_id, u32 *sub_channel_id);
struct prof_sub_channel_info *prof_get_sub_channel_info_from_ctx(struct prof_proc_ctx *proc_ctx,
    u32 device_id, u32 vfid, u32 channel_id);
struct prof_sub_channel_info *prof_get_sub_channel_info_from_index(u32 device_id,
    u32 vfid, u32 channel_id, u32 sub_channel_id);
void prof_sub_resource_info_init(u32 device_id, u32 channel_id,
    u32 sub_channel_id, struct prof_sub_channel_info *sub_resource_info);
void prof_sub_resource_channel_state_init(struct prof_sub_channel_info *sub_resource_info);
struct prof_device_resource_info *prof_get_device_resource_info(u32 device_id);
struct prof_channel_resource_info *prof_get_channel_resource_info(u32 device_id,
    u32 channel_id);
struct prof_sub_channel_info *prof_get_sub_resource_info(u32 dev_id,
    u32 channel_id, u32 sub_channel_id);
int prof_ctx_init(struct prof_proc_ctx *ctx);
void prof_ctx_uninit(struct prof_proc_ctx *ctx);
void prof_free_sub_channel_id(struct prof_sub_channel_res *sub_channel_res,
    u32 device_id, u32 phy_devid, u32 channel_id, u32 sub_channel_id);
int prof_ctx_sub_channel_res_init(struct prof_sub_channel_res *sub_channel_res);
void prof_ctx_sub_channel_res_uninit(struct prof_sub_channel_res *sub_channel_res);
extern u32 devdrv_manager_get_ts_num(struct devdrv_info *dev_info);
void prof_dev_dfx_load(prof_dev_dfx_info_t *dev_dfx, struct prof_ioctl_para *prof_para);
int prof_stop_wait_buff_read_over(struct prof_sub_channel_info *sub_channel_info, enum prof_channel_type type,
    struct prof_proc_ctx *proc_ctx);
int prof_alloc_device_info(u32 device_id);
void prof_free_device_info(u32 device_id);
int prof_each_device_init(u32 device_id, u32 aicore_num, u32 total_core_num);
void prof_each_device_uninit(u32 device_id);
bool prof_is_ts_valid(void);
void prof_init_hash(void);

enum prof_node_mem_type {
    CTRL_NODE_MEM_TYPE,
    TS_NODE_MEM_TYPE,
    NODE_MEM_TYPE_MAX
};

unsigned char *prof_alloc_node_memory(u32 device_id, u32 size, gfp_t flags, enum prof_node_mem_type type);

#endif /* PROF_DRV_DEV_H */
