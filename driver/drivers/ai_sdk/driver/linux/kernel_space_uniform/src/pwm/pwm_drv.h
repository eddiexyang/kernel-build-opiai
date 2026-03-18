/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-10-12
 */

#ifndef _PWM_DRV_H
#define _PWM_DRV_H

#include <linux/cdev.h>
#include <linux/device.h>
#include "drv_log.h"

#ifdef STATIC_SKIP
#define STATIC
#else
#define STATIC static
#endif

#define PWM_LOG_PREFIX "pwm"
#define pwm_log_err(fmt...) drv_err(PWM_LOG_PREFIX, fmt)
#define pwm_log_warn(fmt...) drv_warn(PWM_LOG_PREFIX, fmt)
#define pwm_log_info(fmt...) drv_info(PWM_LOG_PREFIX, fmt)

#define COMM_DEV_CLASS "comm_dev_class"
#define COMM_DEV_NODE_NAME "comm_dev"
#define COMM_DEV_MAX_COUNT 1
#define DEV_PWM_NAME_KER "pwm"

#define DEV_PWM_NAME "/dev/pwm"

#define PWM_IOC_MAGIC 'p'
#define PWM_CMD_GET_DUTY_RATIO _IOR(PWM_IOC_MAGIC, 0, int) // 获取占空比
#define PWM_CMD_SET_DUTY_RATIO _IOW(PWM_IOC_MAGIC, 1, int) // 设置占空比
#define PWM_CMD_GET_DUTY_RATIO_ADJUST_MODE _IOR(PWM_IOC_MAGIC, 2, int) // 获取调速模式
#define PWM_CMD_SET_DUTY_RATIO_ADJUST_MODE _IOW(PWM_IOC_MAGIC, 3, int) // 设置调速模式
#define PWM_CMD_GET_FAN_SPEED _IOW(PWM_IOC_MAGIC, 4, int) // 设置调速模式

#define PWM_PHY_ADDR 0xC4080000   /* PWM寄存器基地址 */
#define IOCFG_PHY_ADDR 0xC4000000 /* AO IOMUX复用寄存器基地址 */
#define PWM_PHY_REG_SIZE 0x1000
#define IOCFG_PHY_REG_SIZE 0x1000
#define EACH_INFO 0x8     // PWM通道0配置寄存器相对基地址起始偏移
#define PWM_IO_ADD 0xBC   /* PWM0管脚的复用控制寄存器 */
#define PWM_MAX_CHANNEL 3 /* PWM支持通道0-3 */
#define CYCLE_NUM 10000
#define FAN_MEASURE_TIME_REG 0x108  // 时间窗设置的寄存器
#define FAN_MEASURE_TIME_VALUE 1999 // 时间窗的大小，250ms
#define IOMUX_PWM_MODE 0            // IO复用使用PWM模式时，对应的寄存器应该配置的值
#define NOT_FIND_CMD 0X1
#define FAN_PWM_CHANNEL 2 // 散热风扇PWM通道

#define PWM_DEFAULT_RATIO 40
#define PWM_MAX_RATIO 100
#define PWM_RATIO_VAL 100
#define PWM_REG_OFFSET 4
#define PWM_REG_OFFSET_MAX 12

#define DMS_LPM_SOC_ID 8
#define DMS_VF_ID_PF 0
#define DMS_LPM_GET_TEMPERATURE 0

#define TEMPERATURE_ADJUST_DISCREPANCY 5
#define THREAD_PAUSE_TIME 100
#define PWM_DUTY_TARIO_LIMIT 1

#define OFFSET_ADDR_PWM_CHEN_REG 0x0 // 脉冲周期寄存器
#define PWM_CHEN_OFF 0x0    // 脉冲周期关
#define PWM_CHEN_ON  0x100F // 脉冲周期开
#define OFFSET_ADDR_PWM_PRD0_REG 0x8 // 0通道脉冲周期寄存器
#define OFFSET_ADDR_PWM_CH0_PWL_REG 0xC // 0通道脉冲低电平起始寄存器
#define OFFSET_ADDR_PWM_PRD0_VALUE 0x0 // 0通道脉冲周期
#define OFFSET_ADDR_FAN_INT_EN_REG 0x104 // 风扇中断使能寄存器
#define OFFSET_ADDR_FAN_INT_EN_VALUE 0xFFFFFFFF // 风扇中断使能值
#define OFFSET_ADDR_FAN_TH0_REG 0xA8 // 风扇0转速下限寄存器
#define OFFSET_ADDR_FAN_TH0_VALUE 0x0 // 风扇0转速下限寄存器
#define OFFSET_ADDR_FAN_SR0_REG 0xA4 // 风扇0转速寄存器

// 计数周期为250ms，结果乘（4*60） 即可获得60秒钟的脉冲个数
// 风扇每转1圈发2个脉冲，计算转速需要除2
#define SCALE_FACTOR (4 * 60 / 2)
#define FAN_SPEED_READY_FLAG 0x100
#define VALID_BITS 0x1FF
#define SLEEP_MS 100
#define NUMBER_OF_CYCLES 30

typedef int (*get_temperature)(void *feature, char *in, u32 in_len, char *out, u32 out_len);

typedef struct {
    unsigned int channel_num; // PWM通道号（0---11）
    unsigned int ratio;       // 占空比（取值范围为：0---100，例：ratio = 45 表示占空比为45%）
    unsigned int speed;       // 风扇转速（r/min）
} PWM_INFO;

typedef struct {
    unsigned int num;
    unsigned int pwm_val;
} PWM_IO_INFO;

struct duty_ratio_adjust_info {
    unsigned int last_curve_flag;
    unsigned int cur_curve_flag;
    unsigned int last_temp;
    unsigned int cur_temp;
};

struct duty_ratio_curve {
    unsigned int curve_flag; // 当前温度变化曲线标记（0：升温，1：降温）
    unsigned int min_temp;
    unsigned int max_temp;
    int curve_k;
    int curve_b;
};

enum pwm_mode {
    PWM_MANUAL,
    PWM_AUTO,
    PWM_MODE_MAX,
};

struct dms_lpm_info_in {
    unsigned int dev_id;
    unsigned int vfid;
    unsigned int core_id;
    unsigned int sub_cmd;
};

typedef struct st_devtype {
    unsigned int dev_major;
    unsigned int dev_minor;
    struct cdev cdev; /* Char device structure */
#ifndef _PWM_UT_
    struct class *dev_class;
#else
    struct _class_ *dev_class;
#endif
} CDEV_ST;

#endif
