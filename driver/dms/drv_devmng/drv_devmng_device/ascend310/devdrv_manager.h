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

#include "devdrv_manager_common.h"
#include "devdrv_manager_comm.h"
#include "devdrv_interface.h"
#include "devdrv_pm.h"
#include "drv_ipc.h"
#include "devdrv_platform_register.h"

#define DEVDRV_FW_TYPE_AICPU 0
#define DEVDRV_FW_TYPE_TS 1

#define DEVDRV_IRQ_LEVEL_TRIGGERED 0x1
#define DEVDRV_IRQ_EDGE_TRIGGERED 0x3

#define DEVDRV_TS_DOORBELL_IRQ 325
#define DEVDRV_TS_NODE_DDR_ID_OFFSET 1

#define DEVDRV_GPIO_TOGGOLE 1
#define DEVDRV_GPIOIRQ_TIMEOUT 3000
#define DEVDRV_GPIOIRQ_THRED_STOP 0x3467

/* wait host send devid to device time, (20*1000)ms */
#define DEVDRV_WAIT_TIME_DEVID          (20 * 1000)
#define DEVDRV_WAIT_TIME_DEVID_ONCE      10

#define TS_STATIC_ADDR_BASE 0x62000000

#define INVAILD_DEVICE_ID 0xff

#define PMU21_VBUCK_VOLTAGE 0
#define PMU21_VOUT_VOLTAGE 1
#define PMU22_VBUCK_VOLTAGE 2
#define PMU_ADC_VOLTAGE 3
#define PMU_MAIN_DIEID 0
#define PMU_SECOND_DIEID 1

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
    u64 mnt_ns;
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

void devdrv_manager_inform_device_status(struct devdrv_info *info, enum devdrv_ts_status status);
void devdrv_flush_cache(u64 base, u32 len);

u64 devdrv_read_cntpct(void);

/*
 * ipc message
 */
struct ipcdrv_msg_payload {
    u8 result;
    u8 reserved[IPCDRV_MSG_LENGTH - 1];
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
/* cmd type0: */
#define LPM3_QUERY_CMD 2
#define LPM3_LOGLEVEL_CMD 3
#define LPM3_NOTIFY_CMD 4
#define LPM3_HEART_BEAT_QUERY 8
/* obj or src id: */
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

/* process white list */
#define WHITE_LIST_PROCESS_NUM 1
#define PROCESS_NAME_DMP "dmp_daemon"
#define PROCESS_NAME_TSD "tsdaemon"

#define WHITE_LIST_PROCESS_NUM_FOR_BIND_PID     1

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

struct ioctl_arg {
    u32 dev_id;
    u32 type;  // core id or device type
    u32 data1; // u32 data
    int data2; // int data  for temperature
    u32 data3;
};

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

enum tagAicpufwPlat {
    AICPUFW_ONLINE_PLAT = 0,
    AICPUFW_OFFLINE_PLAT,
    AICPUFW_MAX_PLAT,
};

#define VFID_NUM_MAX 32  /* virtual function id max num */
struct devdrv_process_sign {
    pid_t hostpid;
    u32 host_process_status;
    pid_t devpid[DEVDRV_MAX_NODE_NUM][VFID_NUM_MAX][DEVDRV_PROCESS_CPTYPE_MAX];
    char sign[PROCESS_SIGN_LENGTH];
    u32 cp_count; /* count cp1 & dev_only devpid num for releasing sign */
    u32 in_use_count;
    struct hlist_node link; /* hash find link */
};

/* register operation */
#define DEVDRV_REG_WR 0
#define DEVDRV_REG_RD 1

int devdrv_reg_op(unsigned char op_type, unsigned long base_phy_reg, unsigned long reg_offset,
    unsigned long map_size, unsigned int *val);

int devdrv_manager_inquiry_info(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_inquiry_info_ex(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_imu_cmd(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_get_pcie_id_info(u32 devid, struct dmanage_pcie_id_info *pcie_id_info);
u32 devdrv_manager_get_devid(u32 local_devid);
struct devdrv_info *devdrv_get_default_devdrv_info(void);

int devdrv_black_box_init(void);
void devdrv_black_box_exit(void);

void devdrv_manage_wake_up_msg_poll(void);
int devdrv_manager_get_pmu_voltage(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_manager_get_pmu_dieid(struct file *filep, unsigned int cmd, unsigned long arg);
struct devdrv_manager_info *devdrv_get_manager_info(void);
u32 devdrv_manager_get_devnum(void);

void devdrv_os_heart_beat_init(void);
int drv_ipc_msg_request(struct file *filep, unsigned int cmd, unsigned long arg);
void devdrv_manager_ops_sem_down_write(void);
void devdrv_manager_ops_sem_up_write(void);
void devdrv_manager_ops_sem_down_read(void);
void devdrv_manager_ops_sem_up_read(void);
int devdrv_manager_register(struct devdrv_info *dev_info);
void devdrv_manager_unregister(struct devdrv_info *dev_info);
struct tsdrv_drv_ops *devdrv_manager_get_drv_ops(void);

int devmng_get_vdavinci_info(u32 vdev_id, u32 *phy_id, u32 *vfid);
int hvdevmng_get_aicore_num(u32 devid, u32 fid, u32 *aicore_num);
void devdrv_fresh_error_code_to_shm(struct work_struct *work);

int devdrv_fresh_event_code_to_shm(u32 devid, u32 *health_code, u32 health_len,
    struct shm_event_code *event_code, u32 event_len);
int dms_device_register(struct devdrv_info* dev);
void dms_device_unregister(struct devdrv_info* dev);
int devdrv_lpm3_notifier_handle(u32 dev_id, unsigned long len, void *data);
int devdrv_lpm3_notifier_chan2(u32 dev_id, unsigned long len, void *data);
int devdrv_manager_send_tslog_addr_to_host(u32 devid, u64 phy_addr, u32 mem_size, bool dynamic_alloc);

#endif /* __DEVDRV_MANAGER_H */
