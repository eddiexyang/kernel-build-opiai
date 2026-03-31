/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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

#ifndef __DEVDRV_MANAGER_H
#define __DEVDRV_MANAGER_H
#include <linux/securec.h>
#include "devdrv_manager_common.h"
#include "devdrv_manager_comm.h"
#include "devdrv_interface.h"
#include "devdrv_pm.h"
#include "drv_ipc.h"
#include "devdrv_platform_register.h"
#include "devdrv_black_box.h"

#define DEVDRV_AICORE_FREQ 1000

#define DEVDRV_IRQ_LEVEL_TRIGGERED 0x1
#define DEVDRV_IRQ_EDGE_TRIGGERED 0x3

#define DEVDRV_TS_DOORBELL_IRQ 325
#if (defined CFG_SOC_PLATFORM_CLOUD)
#define DEVDRV_TS_NODE_DDR_ID_OFFSET 3
#define DEVDRV_HBM_ID_OFFSET         2
#elif (defined CFG_SOC_PLATFORM_MINIV2)
#define DEVDRV_TS_NODE_DDR_ID_OFFSET 2
#define DEVDRV_HBM_ID_OFFSET         0
#else
#define DEVDRV_TS_NODE_DDR_ID_OFFSET 0
#define DEVDRV_HBM_ID_OFFSET         0
#endif

#define DEVDRV_GPIO_TOGGOLE 1
#define DEVDRV_GPIOIRQ_TIMEOUT 3000
#define DEVDRV_GPIOIRQ_THRED_STOP 0x3467

#define DEVDRV_WAIT_TIME_FPGA_DC      3000000
#define DEVDRV_WAIT_TIME_NORMAL      300000
#define DEVDRV_WAIT_TIME_SHORT_TIME      50

/* wait host send devid to device time, (20*1000)ms */
#define DEVDRV_WAIT_TIME_DEVID         (20*1000)
#define DEVDRV_WAIT_TIME_DEVID_ONCE     10


#define INVAILD_DEVICE_ID 0xff

#define PMU21_VBUCK_VOLTAGE 0
#define PMU21_VOUT_VOLTAGE 1
#define PMU22_VBUCK_VOLTAGE 2
#define PMU_ADC_VOLTAGE 3
#define PMU_MAIN_DIEID 0
#define PMU_SECOND_DIEID 1

#define DEVDRV_REQ_ENABLE_BIT 0x1
#define DEVDRV_DATA_ENABLE_BIT 0x2
#define DEVDRV_L3T_REG_NUM 0x4
#define DEVDRV_L3T_REG_OFFSEF 0x414
#define DEVDRV_L3_TAG0_REG 0x89060000
#define DEVDRV_L3_TAG1_REG 0x8B060000
#define DEVDRV_L3_TAG2_REG 0x89070000
#define DEVDRV_L3_TAG3_REG 0x8B070000

#define DEVDRV_CHIP_ADDR_OFFSET 0x200000000000
#define DEVDRV_ADDR_DEVMNG_OFFSET 0x22140000
#define DEVDRV_ADDR_DEVMNG_SIZE 0x1000000

/* add for partial good */
#ifdef CFG_SOC_PLATFORM_CLOUD
#define EFUSE_BASE_ADDR 0x8B340000
#define EFUSE_AICORE_NUM_OFFSET     0xE2B8
#define EFUSE_AICORE_FREQ_OFFSET    0xE224
#define SIZE_OF_64K 0x10000
#define EFUSE_SIZE 4
#define PARTIAL_GOOD_CORE_NUM 30
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#define FULL_GOOD_CORE_NUM 48
#else
#define FULL_GOOD_CORE_NUM 32
#endif
#define PARTIAL_GOOD_FREQ_LITE      900
#define PARTIAL_GOOD_FREQ_MEDIUM    1000
#define PARTIAL_GOOD_FREQ_PRO       1200
#define PARTIAL_GOOD_FREQ_EFUSE_VAL_ZERO    0
#define PARTIAL_GOOD_FREQ_EFUSE_VAL_ONE     1
#define PARTIAL_GOOD_FREQ_EFUSE_VAL_THREE   3
#define PARTIAL_GOOD_FREQ_EFUSE_VAL_FIVE    5
#define PARTIAL_GOOD_EFUSE_OFFSET_NINE      9
#define PARTIAL_GOOD_EFUSE_AND_SEVEN   0x07
#endif

/* process white list */
#define WHITE_LIST_PROCESS_NUM 1
#define PROCESS_NAME_DMP "dmp_daemon"
#define PROCESS_NAME_TSD "tsdaemon"

#define WHITE_LIST_PROCESS_NUM_FOR_BIND_PID     1

struct devdrv_gpioirq {
    int pid;
    int tgid;
    int gpio;
    int gpioirq_trigger;
    int gpioirq_count;

    int thread_stop;

    struct semaphore sem;
    wait_queue_head_t wait;
};

struct ipc_notify_info {
    u32 open_fd_num;
    u32 create_fd_num;

    /* created node list head */
    struct list_head create_list_head;
    struct list_head open_list_head;

    struct mutex info_mutex;
};

struct devdrv_manager_context {
    int pid;
    int tgid;
    struct mnt_namespace *mnt_ns;
    struct pid_namespace *pid_ns;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
    u64 start_time;
    u64 real_start_time;
#else
#ifdef DEVDRV_MANAGER_HOST_UT_TEST
    u64 start_time;
    u64 real_start_time;
#else
    struct timespec start_time;
    struct timespec real_start_time;
#endif
#endif

    struct device *dev;
    struct devdrv_gpioirq *gpioirq;
    struct task_struct *task;
    struct ipc_notify_info *ipc_notify_info;
};

/*
 * ipc message
 */
#ifdef CFG_FEATURE_IPC_CRC
# define IPC_MSG_RESERVE_LENGTH (IPCDRV_MSG_LENGTH - 4)
#else
# define IPC_MSG_RESERVE_LENGTH (IPCDRV_MSG_LENGTH - 2)
#endif

struct ipcdrv_msg_payload {
    u8 result;
    u8 vfid;
    u8 reserved[IPC_MSG_RESERVE_LENGTH];
#ifdef CFG_FEATURE_IPC_CRC
    u16 crc;
#endif
};

enum dev_id_type {
    DEVICE_ID0 = 0,
    DEVICE_ID1 = 1,
    DEVICE_ID2 = 2,
    DEVICE_ID3 = 3,
    DEVICE_ID_MAX,
};

/* cmd type1: */
#define LPM3_TEMPERATURE 11
#define LPM3_VOLTAGE 12
#define LPM3_HEART_BEAT 25
#define LPM3_AICORE_FREQ 26
#define LPM3_DDR_FREQ 27
#define LPM3_DDR_VOLUME 28
#define LPM3_SUGGEST_LIMIT 29
#define LPM3_CANCEL_LIMIT 30
#define LPM3_TSENSOR_RESET_ALARM 31
#define LPM3_CANCEL_RESET_ALARM 32
#define LPM3_EXCEPTION_INFO 34
#define LPR52_TYPE1 0xa
/* cmd type0: */
#define LPM3_QUERY_CMD 2
#define LPM3_LOGLEVEL_CMD 3
#define LPM3_NOTIFY_CMD 4
#define LPM3_HEART_BEAT_QUERY 8
#define LPR52_LOGLEVEL_CMD 0x2
/* obj or src id: */
#define LPR52_SOURECE_ID 0
#define LPR52_TARGET_ID 0x4
#define LPM3_HEART_ID 3
#define LPM3_PSCI_ID 10
#define LPM3_ID 2
#define DEVICE_CCPU_ID 1
#define LPM3_SMOKE 24
#define LPM3_THRESHOLD 32
#define LPM3_EDP 36

#define LPM3_LOG_ID 35
/* cmd para[0] */
#define LPM3_UPPER_FREQ 1
#define LPM3_LOWER_FREQ 0

struct devdrv_ipc_lpm3 {
    u8 cmd_type1;
    u8 cmd_type0;
    u8 target_id;
    u8 source_id;
    u8 cmd_para0;
    u8 cmd_para1;
    u8 cmd_para2;
    u8 cmd_para3;
};

struct devdrv_lpm3_heart_beat {
    u8 cmd_type1;
    u8 cmd_type0;
    u8 target_id;
    u8 source_id;
    u8 cmd_para0;
    u8 cmd_para1;
    u8 cmd_para2;
    u8 cmd_para3;

    u64 tv_sec;
    u64 tv_usec;
};

struct devdrv_lpm3_exception_info {
    u8 cmd_type1;
    u8 cmd_type0;
    u8 target_id;
    u8 source_id;

    u32 exception_code;

    u64 tv_sec;
    u64 tv_usec;
};

struct _vrd {
    char vrd_temp_0;
    char vrd_temp_1;
    char vrd_temp_2;
    char res;
};

struct ioctl_arg {
    u32 dev_id;
    u32 type;   // core id or device type
    u32 data1;  // u32 data
    int data2;  // int data  for temperature
    u32 data3;
    u32 vfid;
};

struct all_temp {
    char aicore0_temp;
    char aicore1_temp;
    char aicore2_temp;
    char aicore3_temp;
    char aicore4_temp;
    char aicore5_temp;
    char aicore6_temp;
    char aicore7_temp;
    char aicore8_temp;
    char aicore9_temp;
    char cpu0_temp;
    char cpu1_temp;
    char cpu2_temp;
    char cpu3_temp;
    char dvpp_temp;
    char io_temp;
    char ao_temp;
    char isp_temp;
};

#define DMANAGE_ALL_TEMP_LEN 18
struct ioctl_all_temp_arg {
    unsigned int cnt_check;
    union {
        struct all_temp tmp;
        char tmpp[DMANAGE_ALL_TEMP_LEN];
    } data;
};

#define CNT_CHECK_FAIL 2
#define CNT_CHECK_SUCC 1
#define DDR_ALL_TEMP_ADDR 0XA20000
#define DDR_ALL_TEMP_SIZE 0X1000
#define DDR_CNT_CHECK_OFFSET 60

struct bb_err_string {
    unsigned int dev_id;
    unsigned int errcode;
    int buf_len;
    unsigned char errstr[BBOX_ERRSTR_LEN];
};

struct pmu_voltage_stru {
    unsigned int pmu_type;
    unsigned int device_id;
    unsigned int channel;
    unsigned int get_value;
    int return_value;
};
#define PMU_DIEID_GET_VALUE 8

struct pmu_dieid_stru {
    unsigned int pmu_dieid_type;
    unsigned int device_id;
    unsigned char len;
    unsigned int get_value[PMU_DIEID_GET_VALUE];
};

typedef enum {
    TEMP_SUB_CMD_DDR_THOLD = 1,
    TEMP_SUB_CMD_SOC_THOLD,
    TEMP_SUB_CMD_SOC_MIN_THOLD,
    TEMP_SUB_CMD_MAX,
} TEMP_SUB_CMD;

typedef enum {
    VDEV_BIND = 0,
    VDEV_UNBIND,
    VDEV_INVALID_ACTION,
} vdev_action;

typedef enum {
    ALSO_DOES_SUPPORT_VF,
    ONLY_DOES_SUPPORT_PF,
    TRANS_PHYID_TO_PFID,
} pfvf_deal_flag;

int devdrv_manager_trans_and_check_id(u32 logical_dev_id, u32 *physical_dev_id, u32 *vfid, pfvf_deal_flag deal);
int devdrv_manager_inquiry_info(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_imu_cmd(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_inquiry_info_ex(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_get_pcie_id_info(u32 devid, struct dmanage_pcie_id_info *pcie_id_info);
int devdrv_manager_get_device_index(u32 host_dev_id, u32 *local_dev_id);
u32 devdrv_manager_get_devid(u32 local_devid);
struct devdrv_info *devdrv_get_default_devdrv_info(void);

void devdrv_manage_wake_up_msg_poll(void);
int devdrv_manager_get_pmu_voltage(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_get_pmu_dieid(struct file *filep, unsigned int cmd, unsigned long arg);
u32 devdrv_manager_get_devnum(void);
#ifdef CFG_FEATURE_OLD_DEVID_TRANS
u32 devdrv_manager_get_vdevnum(void);
#endif
struct devdrv_manager_info *devdrv_get_manager_info(void);
void devdrv_enable_irq(struct devdrv_info *dev_info, u32 irq);
int devdrv_map_register_memory(unsigned int dev_id);
void devdrv_unmap_register_memory(unsigned int dev_id);
int devdrv_agent_sync_msg_send(u32 dev_id, struct devdrv_manager_msg_info *msg_info, u32 payload_len, u32 *out_len);
struct tsdrv_drv_ops *devdrv_manager_get_drv_ops(void);
void devdrv_mn_mutex_lock(int dev_id);
void devdrv_mn_mutex_unlock(int dev_id);
void devdrv_manager_inform_device_status(struct devdrv_info *info, enum devdrv_ts_status status);
int devdrv_get_ffts_type(unsigned int *op_val);
struct devdrv_info *devdrv_manager_get_devdrv_info(u32 dev_id);

#if defined(CFG_SOC_PLATFORM_MINI) && !defined(CFG_SOC_PLATFORM_MINIV2)
int pmu_ldo2_enable(void);
int pmu_ldo2_disable(void);
#endif

#if defined(CFG_SOC_PLATFORM_MINI) || defined(CFG_SOC_PLATFORM_CLOUD)
#if !defined(CFG_SOC_PLATFORM_MINIV2)
int hisi_adc_get_value(unsigned int channel);
int get_second_pmu_buck_volt(unsigned int device_id, unsigned int channel, unsigned int *volt_mv);
int get_main_pmu_buck_volt(unsigned int channel, unsigned int *volt_mv);
int get_main_pmu_ldo_volt(unsigned int channel, unsigned int *volt_mv);
#endif
#endif

#ifdef CFG_SOC_PLATFORM_MINI
int devdrv_get_config_index_by_name(const char *name);
s32 devdrv_get_user_config_core(u32 dev_id, s32 cfg_index, u8 *buf, u32 *buf_size);
#endif

int devdrv_gpioirq_register(struct file *filep, unsigned int cmd, unsigned long arg);
void devdrv_gpioirq_unregister(struct devdrv_manager_context *dev_manager_context);
int devdrv_gpioirq_wait_int(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_get_boardid(void);
u32 devdrv_manager_get_ts_num(struct devdrv_info *dev_info);

#ifdef CFG_SOC_PLATFORM_CLOUD
extern struct devdrv_cpu_info g_cpu_info[MAX_CHIP_NUM];
int devdrv_get_tsdrv_cq_aisle_irq(int node_id);
#elif defined(CFG_SOC_PLATFORM_MINI)
#define MAX_HEARTBEAT_LOSS (2)
extern u32 g_agentdrv_doorbell_irq_cnt;
extern void sysrq_sched_debug_show_export(void);
#else
#endif

#ifdef CFG_FEATURE_HISS
int devdrv_manager_get_flash_info(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_get_hiss_status(struct file *filep, unsigned int cmd, unsigned long arg);
#endif
#if defined(CFG_SOC_PLATFORM_MINIV2)
extern struct devdrv_cpu_info g_cpu_info[MAX_CHIP_NUM];
int devdrv_manager_set_power_state(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_get_ufs_status(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_get_ufs_info(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_set_ufs_info(struct file *filep, unsigned int cmd, unsigned long arg);
#ifdef CFG_SOC_PLATFORM_MDC_V51
int devdrv_manager_get_emu_subsys_status(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_get_safetyisland_status(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_safetyisland_notifier(struct notifier_block *nb, unsigned long len, void *data);
int devdrv_manager_get_power_state(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_equipment_set_safety_island_info(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_equipment_get_safety_island_info(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_get_sils_info(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_set_sils_info(struct file *filep, unsigned int cmd, unsigned long arg);
#endif
#ifdef CFG_FEATURE_REBOOT_REASON
void devdrv_manager_record_reset_reason(unsigned int addr_offset, unsigned int reason);
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V11
int devdrv_manager_set_power_state_v2(struct file *filep, unsigned int cmd, unsigned long arg);
#endif
int devdrv_manager_get_ts_group_num(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_get_capability_group_info(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_delete_capability_group(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_create_capability_group(struct file *filep, unsigned int cmd, unsigned long arg);
#endif
void devdrv_lpm_exception_info(void *data, unsigned long len);
void devdrv_os_heart_beat_init(void);
int hvdevmng_get_aicore_num(u32 devid, u32 fid, u32 *aicore_num);
/* stub for device container.c */
int devmng_get_vdavinci_info(u32 vdev_id, u32 *phy_id, u32 *vfid);
int dev_mnt_vdevice_add_inform(unsigned int vdev_id,
    vdev_action action, struct mnt_namespace *ns, u64 container_id);
void dev_mnt_vdevice_inform(void);

int drv_ipc_msg_request(struct file *filep, unsigned int cmd, unsigned long arg);
void devdrv_manager_ops_sem_down_write(void);
void devdrv_manager_ops_sem_up_write(void);
void devdrv_manager_ops_sem_down_read(void);
void devdrv_manager_ops_sem_up_read(void);
int devdrv_manager_register(struct devdrv_info *dev_info);
void devdrv_manager_unregister(struct devdrv_info *dev_info);
#ifdef CFG_FEATURE_CHIP_DIE
int devdrv_manager_get_random_from_bar(u32 devid, char *random_number, u32 random_len);
#endif

int dms_device_register(struct devdrv_info* dev);
void dms_device_unregister(struct devdrv_info* dev);
/* call for drv_memory */
int devdrv_get_ecc_statistics(unsigned long arg);
int devdrv_get_freq_from_lp_memory(unsigned int dev_id, unsigned int type, unsigned int *freq);
int devdrv_query_ddr_statistic_from_lp(unsigned int dev_id, unsigned int type, void *out_msg);
int devdrv_manager_get_hw_info(struct devdrv_info *dev_info);
int devdrv_manager_send_tslog_addr_to_host(u32 devid, u64 phy_addr,
    u32 mem_size, bool dynamic_alloc);

#endif /* __DEVDRV_MANAGER_H */
