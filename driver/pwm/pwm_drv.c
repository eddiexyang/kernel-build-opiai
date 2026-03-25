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

#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/kallsyms.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include "pwm_drv.h"

STATIC int pwm_open(struct inode *pnode, struct file *pfile);
STATIC long pwm_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg);
STATIC int pwm_release(struct inode *pnode, struct file *pfile);

CDEV_ST *g_infra_pwm_dev = NULL; /* pwm控制器字符设备属性指针 */

/* 用于map寄存器空间 */
unsigned long g_infra_pwm_map_addr = 0;
unsigned long g_iocfg_add = 0;

atomic_t g_pwm_mode;
unsigned int g_pwm_duty_ratio;
struct task_struct *g_adjust_duty_ratio_thread = NULL;

/* 文件操作结构体 */
struct file_operations g_infra_pwm_fops = {
    .owner = THIS_MODULE,
    .open = pwm_open,
    .release = pwm_release,
    .unlocked_ioctl = pwm_ioctl,
};

STATIC int pwm_open(struct inode *pnode, struct file *pfile)
{
    if (pnode == NULL || pfile == NULL) {
        pwm_log_err("Invalid parameter. pnode or pfile is NULL\n");
        return -EINVAL;
    }

    if (g_infra_pwm_dev != NULL) {
        pfile->private_data = g_infra_pwm_dev;
    }

    return 0;
}

STATIC int pwm_set_duty_ratio(const PWM_INFO *pwm_info)
{
    unsigned int offset;
    unsigned int base;
    int ratio;

    if (pwm_info->channel_num > PWM_MAX_CHANNEL) {
        pwm_log_err("pwm channel %u out of range", pwm_info->channel_num);
        return -EINVAL;
    }
    if (pwm_info->ratio > PWM_MAX_RATIO) {
        pwm_log_err("pwm ratio %u out of range", pwm_info->ratio);
        return -EINVAL;
    }

    // 设置脉冲周期
    base = EACH_INFO;
    offset = base + ((pwm_info->channel_num) * PWM_REG_OFFSET_MAX);
    iowrite32(CYCLE_NUM, (char *)(uintptr_t)(g_infra_pwm_map_addr + offset));

    // 设置低电平开始时刻
    offset = offset + PWM_REG_OFFSET;
    iowrite32(0, (char *)(uintptr_t)(g_infra_pwm_map_addr + offset));

    // 设置高电平开始时刻
    offset = offset + PWM_REG_OFFSET;
    ratio = pwm_info->ratio * PWM_RATIO_VAL;
    // 高电平开始时刻最小值为100
    ratio = (ratio == 0 ? 100 : ratio);
    iowrite32(ratio, (char *)(uintptr_t)(g_infra_pwm_map_addr + offset));

    return 0;
}

STATIC int pwm_get_duty_ratio(PWM_INFO *pwm_info)
{
    unsigned int offset;
    unsigned int base;
    unsigned int high_level_time; // 高电平开始的时刻

    if (pwm_info->channel_num > PWM_MAX_CHANNEL) {
        pwm_log_err("pwm channel %u out of range", pwm_info->channel_num);
        return -EINVAL;
    }

    base = EACH_INFO;
    offset = base + ((pwm_info->channel_num) * PWM_REG_OFFSET_MAX);
    offset = offset + PWM_REG_OFFSET + PWM_REG_OFFSET;
    high_level_time = ioread32((char *)(uintptr_t)(g_infra_pwm_map_addr + offset));

    high_level_time = high_level_time / PWM_RATIO_VAL;

    pwm_info->ratio = high_level_time;
    return 0;
}

STATIC int pwm_get_speed(PWM_INFO *pwm_info)
{
    unsigned int offset;
    unsigned int pulsing_reg;
    unsigned int pulsing_value;
    unsigned int fan_speed;
    unsigned int fan_speed_read_count = 0;

    offset = OFFSET_ADDR_FAN_SR0_REG;
    while (1) {
        // 获取寄存器值
        pulsing_reg = ioread32((char *)(uintptr_t)(g_infra_pwm_map_addr + offset));
        // 截取bit[8-0]的有效值
        pulsing_reg &= VALID_BITS;
        // 检测标记位，1数据准备完成，0数据未准备完成
        if ((pulsing_reg & FAN_SPEED_READY_FLAG) == FAN_SPEED_READY_FLAG) {
            // 减掉标记位
            pulsing_value = pulsing_reg - FAN_SPEED_READY_FLAG;
            fan_speed = SCALE_FACTOR * pulsing_value;
            break;
        } else {
            fan_speed_read_count++;
        }

        msleep(SLEEP_MS);

        if (fan_speed_read_count > NUMBER_OF_CYCLES) { // 重复30次
            fan_speed = 0;
            break;
        }
    }

    pwm_info->speed = fan_speed;
    return 0;
}

STATIC int set_pwm_iocfg(const PWM_IO_INFO *pwm_io_info)
{
    unsigned int offset;
    unsigned int skip_pmpwm_offset;
    const unsigned int pmpwm_size = 8; // PMPWM0+PMPWM1寄存器大小为8个字节

    if (pwm_io_info->num > PWM_MAX_CHANNEL) {
        pwm_log_err("pwm num %u out of range", pwm_io_info->num);
        return -EINVAL;
    }

    // 设置复用关系：AO IOMUX基地址 + PWM通道管脚复用控制寄存器偏移地址
    skip_pmpwm_offset = (pwm_io_info->num) > 1 ? pmpwm_size : 0; // 跳过PMPWM寄存器偏移
    offset = PWM_IO_ADD + ((pwm_io_info->num) * PWM_REG_OFFSET) + skip_pmpwm_offset;
    iowrite32(pwm_io_info->pwm_val, (char *)(uintptr_t)(g_iocfg_add + offset));
    return 0;
}

STATIC long pwm_ioctl_get_duty_ratio(struct file *pfile, unsigned long arg)
{
    long ret;
    PWM_INFO pwm_info = { 0 };
    PWM_IO_INFO pwm_io_info = { 0 };
    void *arg_temp = (void *)(uintptr_t)arg;

    if (arg_temp == NULL) {
        pwm_log_err("arg_temp is NULL\n");
        return -EINVAL;
    }

    if (copy_from_user(&pwm_info, arg_temp, sizeof(pwm_info))) {
        pwm_log_err("copy_from_user failed\n");
        return -EINVAL;
    }

    if (pwm_info.channel_num > PWM_MAX_CHANNEL) {
        pwm_log_err("pwm channel %u out of range\n", pwm_info.channel_num);
        return -EINVAL;
    }

    pwm_io_info.num = pwm_info.channel_num;
    pwm_io_info.pwm_val = IOMUX_PWM_MODE;
    ret = set_pwm_iocfg((const PWM_IO_INFO *)&pwm_io_info);
    if (ret < 0) {
        pwm_log_err("set pwm mode failed! ret=%ld\n", ret);
        return ret;
    }

    ret = pwm_get_duty_ratio(&pwm_info);
    if (ret < 0) {
        pwm_log_err("get duty ratio failed! ret=%ld\n", ret);
        return ret;
    }

    ret = pwm_get_speed(&pwm_info);
    if (ret < 0) {
        pwm_log_err("get duty ratio failed! ret=%ld\n", ret);
        return ret;
    }

    if (pwm_info.ratio <= PWM_DUTY_TARIO_LIMIT) {
        pwm_info.ratio = g_pwm_duty_ratio;
    }

    if (copy_to_user(arg_temp, &pwm_info, sizeof(PWM_INFO))) {
        pwm_log_err("copy_to_user failed\n");
        return -EINVAL;
    }

    pwm_log_info("channel: %u ratio: %u\n", pwm_info.channel_num, pwm_info.ratio);
    return 0;
}

STATIC long pwm_ioctl_set_duty_ratio(struct file *pfile, unsigned long arg)
{
    long ret;
    PWM_INFO pwm_info = { 0 };
    PWM_IO_INFO pwm_io_info = { 0 };
    void *arg_temp = (void *)(uintptr_t)arg;

    if (arg_temp == NULL) {
        pwm_log_err("arg_temp is NULL\n");
        return -EINVAL;
    }

    if (copy_from_user(&pwm_info, arg_temp, sizeof(PWM_INFO))) {
        pwm_log_err("copy_from_user failed\n");
        return -EINVAL;
    }
    if (pwm_info.channel_num > PWM_MAX_CHANNEL) {
        pwm_log_err("pwm channel %u out of range\n", pwm_info.channel_num);
        return -EINVAL;
    }

    pwm_io_info.num = pwm_info.channel_num;
    pwm_io_info.pwm_val = IOMUX_PWM_MODE;
    ret = set_pwm_iocfg((const PWM_IO_INFO *)&pwm_io_info);
    if (ret < 0) {
        pwm_log_err("set pmw mode failed! ret=%ld", ret);
        return ret;
    }

    ret = pwm_set_duty_ratio((const PWM_INFO *)&pwm_info);
    if (ret < 0) {
        pwm_log_err("set duty ratio failed! ret=%ld\n", ret);
        return ret;
    }

    g_pwm_duty_ratio = pwm_info.ratio;
    pwm_log_info("channel: %u ratio: %u\n", pwm_info.channel_num, pwm_info.ratio);
    return 0;
}

STATIC long pwm_ioctl_set_duty_ratio_adjust_mode(struct file *pfile, unsigned long arg)
{
    int mode;
    void *arg_temp = (void *)(uintptr_t)arg;

    if (arg_temp == NULL) {
        pwm_log_err("Parameter arg_temp is NULL.\n");
        return -EINVAL;
    }

    if (copy_from_user(&mode, arg_temp, sizeof(mode))) {
        pwm_log_err("Copy from user failed.\n");
        return -EINVAL;
    }

    if (mode < 0 || mode >= PWM_MODE_MAX) {
        pwm_log_err("Invalid Parameter. (mode=%d)\n", mode);
        return -EINVAL;
    }

    atomic_set(&g_pwm_mode, mode);
    pwm_log_info("Set duty ratio adjust mode success. (mode=%d)\n", mode);
    return 0;
}

STATIC long pwn_ioctl_get_duty_ratio_adjust_mode(struct file *pfile, unsigned long arg)
{
    int mode;
    void *arg_temp = (void *)(uintptr_t)arg;

    if (arg_temp == NULL) {
        pwm_log_err("Parameter arg_temp is NULL.\n");
        return -EINVAL;
    }

    mode = atomic_read(&g_pwm_mode);
    if (copy_to_user(arg_temp, &mode, sizeof(mode))) {
        pwm_log_err("Copy to user failed.\n");
        return -EINVAL;
    }

    pwm_log_info("Get duty ratio adjust mode success. (mode=%d)\n", mode);
    return 0;
}

STATIC long pwm_ioctl_get_fan_speed(struct file *pfile, unsigned long arg)
{
    pwm_log_info("pwm_ioctl_get_fan_speed.\n");
    return 0;
}

STATIC long pwm_ioctl_out_interface(struct file *pfile, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        /* 获取风扇转速比(排空比) */
        case PWM_CMD_GET_DUTY_RATIO:
            return pwm_ioctl_get_duty_ratio(pfile, arg);
        /* 设置每个通道的脉冲初始值以及高低电平开始的时刻，通过排空比来设置风扇转速 */
        case PWM_CMD_SET_DUTY_RATIO:
            return pwm_ioctl_set_duty_ratio(pfile, arg);
        case PWM_CMD_GET_DUTY_RATIO_ADJUST_MODE:
            return pwn_ioctl_get_duty_ratio_adjust_mode(pfile, arg);
        case PWM_CMD_SET_DUTY_RATIO_ADJUST_MODE:
            return pwm_ioctl_set_duty_ratio_adjust_mode(pfile, arg);
        case PWM_CMD_GET_FAN_SPEED:
            return pwm_ioctl_get_fan_speed(pfile, arg);
        default:
            return NOT_FIND_CMD;
    }
}

STATIC void cleanup_dev(CDEV_ST *pDev)
{
    dev_t dev_no;

    if (pDev == NULL || pDev->dev_class == NULL) {
        pwm_log_err("pDev or dev_class is NULL\n");
        return;
    }

    dev_no = MKDEV(pDev->dev_major, pDev->dev_minor);
    cdev_del(&pDev->cdev);
    device_destroy(pDev->dev_class, dev_no);
    class_destroy(pDev->dev_class);
    unregister_chrdev_region(dev_no, COMM_DEV_MAX_COUNT);

    return;
}

STATIC int init_pwm_dev(CDEV_ST *pDev, struct file_operations *pfoprs)
{
    dev_t dev_no;
    int ret;
    struct device *dev = NULL;
#ifndef _PWM_UT_
    struct class *dev_class = NULL;
#else
    struct _class_ *dev_class = NULL;
#endif

    if ((pDev == NULL) || (pfoprs == NULL)) {
        pwm_log_err("pDev or pfoprs is NULL\n");
        return -EINVAL;
    }

    pDev->dev_class = NULL;

    ret = alloc_chrdev_region(&dev_no, 0, COMM_DEV_MAX_COUNT, DEV_PWM_NAME_KER);
    if (ret < 0) {
        pwm_log_err("alloc_chrdev_region for %s ret: %d\n", DEV_PWM_NAME_KER, ret);
        return ret;
    }

    dev_class = class_create(THIS_MODULE, COMM_DEV_CLASS);
    if (IS_ERR(dev_class)) {
        pwm_log_err("class_create %s error\n", COMM_DEV_CLASS);
        ret = PTR_ERR(dev_class);
        goto exit_unregister_chrdev_region;
    }

    dev = device_create(dev_class, NULL, dev_no, NULL, DEV_PWM_NAME_KER);
    if (IS_ERR(dev)) {
        pwm_log_err("device_create %s error\n", DEV_PWM_NAME_KER);
        ret = PTR_ERR(dev);
        goto exit_class_destroy;
    }

    cdev_init(&pDev->cdev, pfoprs);
    pDev->cdev.owner = THIS_MODULE;
    ret = cdev_add(&pDev->cdev, dev_no, 1);
    if (ret) {
        pwm_log_err("cdev_add %s error\n", DEV_PWM_NAME_KER);
        goto exit_device_create;
    }

    pDev->dev_class = dev_class;
    pDev->dev_major = MAJOR(dev_no);
    pDev->dev_minor = MINOR(dev_no);

    return 0;

exit_device_create:
    device_destroy(dev_class, dev_no);
exit_class_destroy:
    class_destroy(dev_class);
exit_unregister_chrdev_region:
    unregister_chrdev_region(dev_no, COMM_DEV_MAX_COUNT);

    return ret;
}

STATIC long pwm_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
{
    long ret;

    if (pfile == NULL) {
        pwm_log_err("Invalid parameter. pfile is NULL\n");
        return -EINVAL;
    }

    ret = pwm_ioctl_out_interface(pfile, cmd, arg);
    if (ret != NOT_FIND_CMD) {
        return ret;
    }

    pwm_log_err("cmd invalid. cmd = 0x%x\n", cmd);
    return -EINVAL;
}

STATIC int pwm_release(struct inode *pnode, struct file *pfile)
{
    if (pnode == NULL || pfile == NULL) {
        pwm_log_err("Invalid parameter. pnode or pfile is NULL\n");
        return -EINVAL;
    }

    pfile->private_data = NULL;
    return 0;
}

STATIC void __exit pwm_drv_exit(void)
{
    /* 取消内存映射 */
    if (g_infra_pwm_map_addr != 0) {
        iounmap((void __iomem *)(uintptr_t)g_infra_pwm_map_addr);
        g_infra_pwm_map_addr = 0;
    }
    if (g_iocfg_add != 0) {
        iounmap((void __iomem *)(uintptr_t)g_iocfg_add);
        g_iocfg_add = 0;
    }

    /* 字符设备清除 */
    if (g_infra_pwm_dev != NULL) {
        cleanup_dev(g_infra_pwm_dev);
        kfree(g_infra_pwm_dev);
        g_infra_pwm_dev = NULL;
    }

    if (g_adjust_duty_ratio_thread != NULL) {
        (void)kthread_stop(g_adjust_duty_ratio_thread);
        g_adjust_duty_ratio_thread = NULL;
    }

    pwm_log_info("pwm exit ok!\n");
    return;
}

STATIC int match_duty_ratio_curve(unsigned int temp, unsigned int curve_flag, PWM_INFO *pwm_info)
{
    // 温度曲线映射表
    const struct duty_ratio_curve duty_ratio_curve_map[] = {
        {0, 0, 69, 0, 0},
        {0, 69, 70, 15, -1035},
        {0, 70, 80, 1, -55},
        {0, 80, 92, 2, -135},
        {0, 92, 110, 0, 49},
        {1, 0, 64, 0, 0},
        {1, 64, 65, 15, -960},
        {1, 65, 75, 1, -50},
        {1, 75, 87, 2, -125},
        {1, 87, 110, 0, 49},
    };

    size_t index;
    size_t map_size = sizeof(duty_ratio_curve_map) / sizeof(struct duty_ratio_curve);

    for (index = 0; index < map_size; index++) {
        if ((curve_flag == duty_ratio_curve_map[index].curve_flag) && (temp >= duty_ratio_curve_map[index].min_temp) &&
            (temp < duty_ratio_curve_map[index].max_temp)) {
            pwm_info->ratio = duty_ratio_curve_map[index].curve_k * temp + duty_ratio_curve_map[index].curve_b;
            return 0;
        }
    }
    return -EINVAL;
}

STATIC int duty_ratio_computer(struct duty_ratio_adjust_info *adjust_info, PWM_INFO *pwm_info)
{
    int ret = 0;

    if (adjust_info->cur_temp == adjust_info->last_temp) {
        return ret;
    }

    adjust_info->last_curve_flag = adjust_info->cur_curve_flag;
    adjust_info->cur_curve_flag = (adjust_info->cur_temp > adjust_info->last_temp) ? 0 : 1;

    if (adjust_info->cur_curve_flag == adjust_info->last_curve_flag) {
        adjust_info->last_temp = adjust_info->cur_temp;
        ret = match_duty_ratio_curve(adjust_info->cur_temp, adjust_info->cur_curve_flag, pwm_info);
    } else {
        if ((adjust_info->cur_temp > adjust_info->last_temp + TEMPERATURE_ADJUST_DISCREPANCY) ||
            (adjust_info->cur_temp < adjust_info->last_temp - TEMPERATURE_ADJUST_DISCREPANCY)) {
            adjust_info->last_temp = adjust_info->cur_temp;
            ret = match_duty_ratio_curve(adjust_info->cur_temp, adjust_info->cur_curve_flag, pwm_info);
        }
    }

    if (ret != 0) {
        pwm_log_err("Match duty ratio curve failed.\n");
        return -EINVAL;
    }
    return 0;
}

STATIC int adjust_duty_ratio(void *args)
{
    int ret;
    PWM_INFO pwm_info = { 0 };
    struct duty_ratio_adjust_info adjust_info = { 0 };
    get_temperature get_temp_func = NULL;
    int feature = 0;
    unsigned int temperate;
    struct dms_lpm_info_in in;

    get_temp_func = (int (*)(void *, char *, u32, char *, u32))compat_lookup_name("dms_lpm_get_temperature");
    if (get_temp_func == NULL) {
        pwm_log_err("Get dms_lpm_get_temperature function failed.\n");
        return -EINVAL;
    }

    in.dev_id = 0;
    in.vfid = DMS_VF_ID_PF;
    in.core_id = DMS_LPM_SOC_ID;
    in.sub_cmd = DMS_LPM_GET_TEMPERATURE;
    pwm_info.channel_num = 0;

    while (!kthread_should_stop()) {
        if (atomic_read(&g_pwm_mode) == PWM_AUTO) {
            ret = get_temp_func((void *)&feature, (char *)&in, sizeof(in), (char *)&temperate, sizeof(temperate));
            if (ret != 0) {
                pwm_log_err("Get temperature failed. (ret=%d)\n", ret);
                return ret;
            }

            adjust_info.cur_temp = temperate;
            ret = duty_ratio_computer(&adjust_info, &pwm_info);
            if (ret != 0) {
                pwm_log_err("Computer duty ratio failed.\n");
                return ret;
            }
            ret = pwm_set_duty_ratio((const PWM_INFO *)&pwm_info);
            if (ret != 0) {
                pwm_log_err("Set duty ratio failed. (ret=%d)\n", ret);
                return ret;
            }
        }
        msleep(THREAD_PAUSE_TIME);
    }
    return 0;
}

STATIC void pwm_info_init(PWM_IO_INFO *pwm_io_info, PWM_INFO *pwm_info)
{
    pwm_io_info->num = FAN_PWM_CHANNEL;
    pwm_io_info->pwm_val = IOMUX_PWM_MODE;
    (void)set_pwm_iocfg((const PWM_IO_INFO *)pwm_io_info);

    pwm_info->channel_num = FAN_PWM_CHANNEL;
    pwm_info->ratio = PWM_DEFAULT_RATIO;
    (void)pwm_set_duty_ratio((const PWM_INFO *)pwm_info);
}

STATIC void pwm_iounmap(void)
{
    if (g_infra_pwm_map_addr != 0) {
        (void)iounmap((void __iomem *)(uintptr_t)g_infra_pwm_map_addr);
        g_infra_pwm_map_addr = 0;
    }

    if (g_iocfg_add != 0) {
        (void)iounmap((void __iomem *)(uintptr_t)g_iocfg_add);
        g_iocfg_add = 0;
    }
}

STATIC void set_pwm_reg_init(void)
{
    // 关闭PWM
    iowrite32(PWM_CHEN_OFF, (char *)(uintptr_t)(g_infra_pwm_map_addr + OFFSET_ADDR_PWM_CHEN_REG));
    // 设定PWM Period
    iowrite32(OFFSET_ADDR_PWM_PRD0_VALUE, (char *)(uintptr_t)(g_infra_pwm_map_addr + OFFSET_ADDR_PWM_PRD0_REG));
    // 设定PWM低电平开始的点
    iowrite32(OFFSET_ADDR_PWM_PRD0_VALUE, (char *)(uintptr_t)(g_infra_pwm_map_addr + OFFSET_ADDR_PWM_CH0_PWL_REG));
    // 使能中断
    iowrite32(OFFSET_ADDR_FAN_INT_EN_VALUE, (char *)(uintptr_t)(g_infra_pwm_map_addr + OFFSET_ADDR_FAN_INT_EN_REG));
    // 设置风扇的时间窗大小为1999，对应的时间值为250ms
    iowrite32(FAN_MEASURE_TIME_VALUE, (char *)(uintptr_t)(g_infra_pwm_map_addr + FAN_MEASURE_TIME_REG));
    // 使能风扇转速最低下限监控
    iowrite32(OFFSET_ADDR_FAN_TH0_VALUE, (char *)(uintptr_t)(g_infra_pwm_map_addr + OFFSET_ADDR_FAN_TH0_REG));
    // 使能PWM
    iowrite32(PWM_CHEN_ON, (char *)(uintptr_t)(g_infra_pwm_map_addr + OFFSET_ADDR_PWM_CHEN_REG));
}

STATIC int __init pwm_drv_init(void)
{
    int ret;
    PWM_INFO pwm_info = { 0 };
    PWM_IO_INFO pwm_io_info = { 0 };

    g_infra_pwm_dev = (CDEV_ST *)kzalloc(sizeof(CDEV_ST), GFP_KERNEL);
    if (g_infra_pwm_dev == NULL) {
        pwm_log_err(" kzalloc failed!\n");
        ret = -ENOMEM;
        goto out_kmalloc;
    }

    /* 创建设备 */
    ret = init_pwm_dev(g_infra_pwm_dev, &g_infra_pwm_fops);
    if (ret != 0) {
        pwm_log_err("init pwm dev failed! ret=%d\n", ret);
        goto out_kfree;
    }

    /* 物理地址映射虚拟地址 */
    g_infra_pwm_map_addr = (unsigned long)(uintptr_t)ioremap(PWM_PHY_ADDR, PWM_PHY_REG_SIZE);
    if (g_infra_pwm_map_addr == 0) {
        pwm_log_err("ioremap pwm addr failed!\n");
        ret = -ENOMEM;
        goto out_cdev;
    }

    g_iocfg_add = (unsigned long)(uintptr_t)ioremap(IOCFG_PHY_ADDR, IOCFG_PHY_REG_SIZE);
    if (g_iocfg_add == 0) {
        pwm_log_err("ioremap iocfg addr failed!\n");
        ret = -ENOMEM;
        goto out_cdev;
    }

    set_pwm_reg_init();
    atomic_set(&g_pwm_mode, PWM_MANUAL);
    pwm_info_init(&pwm_io_info, &pwm_info);

    g_adjust_duty_ratio_thread = kthread_run(adjust_duty_ratio, NULL, "pwm_thread");
    if (g_adjust_duty_ratio_thread == NULL) {
        pwm_log_err("Create thread failed.\n");
        ret = -EAGAIN;
        goto out_cdev;
    }

    pwm_log_info("pwm init ok!\n");
    return 0;

out_cdev:
    cleanup_dev(g_infra_pwm_dev);
    pwm_iounmap();
out_kfree:
    kfree(g_infra_pwm_dev);
    g_infra_pwm_dev = NULL;
out_kmalloc:
    return ret;
}

module_init(pwm_drv_init);
module_exit(pwm_drv_exit);
MODULE_LICENSE("GPL");
