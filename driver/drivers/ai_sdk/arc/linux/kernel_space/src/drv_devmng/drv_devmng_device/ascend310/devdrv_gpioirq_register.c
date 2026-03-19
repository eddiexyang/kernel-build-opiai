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


#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/kthread.h>

#include "devdrv_manager.h"
#include "devdrv_gpioirq_register.h"
#include "devdrv_i2c_ina2xx.h"

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 13, 0)
#include <uapi/linux/sched/types.h>
#endif

#define TASK_RUN 1
#define TASK_STOP 0

#define BOARDID_EVB_CHANGE 903
#define BOARDID_PCIE_CHANGE 5
#define BOARDID_UNINITED (-1)
#define BOARDID_INIT_FAIL (-2)

int devdrv_boardid = BOARDID_UNINITED;
int devdrv_get_boardid(void)
{
    struct device_node *np = NULL;
    unsigned int boardid[4];
    u32 board_id;
    int ret;
    if (devdrv_boardid == BOARDID_UNINITED) {
        np = of_find_compatible_node(NULL, NULL, "hisilicon,mini");
        if (np == NULL) {
            devdrv_drv_err("failed to find hisilicon,sysctrl node\n");
            devdrv_boardid = BOARDID_INIT_FAIL;
            return -1;
        }

        ret = of_property_read_u32_array(np, "hisi,boardid", boardid, 4);
        if (ret) {
            devdrv_drv_err("failed to read 'hisi,boardid', ret: %d.\n", ret);
            devdrv_boardid = BOARDID_INIT_FAIL;
            return -1;
        }
        board_id = boardid[0] * 1000 + boardid[1] * 100 + boardid[2] * 10 + boardid[3];
        devdrv_boardid = (int)board_id;
    }
    return devdrv_boardid;
}
EXPORT_SYMBOL(devdrv_get_boardid);

bool devdrv_is_pci_rc_mode(void)
{
    int board_id;

    board_id = devdrv_get_boardid();
    if (board_id < 0) {
        devdrv_drv_err("devdrv is rc mode get board id failed, ret:%d\n", board_id);
        return false;
    }

    switch (board_id) {
        case BOARDID_MINI_RC_1:
        case BOARDID_ATLAS_200:
        case BOARDID_MINI_RC_2:
        case BOARDID_MINI_RC_EMMC:
        case BOARDID_MINI_RC_FLASH:
            return true;
        default:
            return false;
    }
}
EXPORT_SYMBOL(devdrv_is_pci_rc_mode);

STATIC u32 devdrv_check_ina2xx(void)
{
    int boardid = devdrv_get_boardid();
    /* ina2xx only used in changed evb board */
    if (boardid >= BOARDID_EVB_CHANGE) {
        return 1;
    }
    return 0;
}

struct devdrv_gpio_state gpio_state;
STATIC int devdrv_lc_is_in_over_current_status(void)
{
    int i;
    for (i = 0; i < DEVDRV_GPIO_LC_CNT; i++) {
        if (gpio_state.state[i].gpio_reg_flag == DEVDRV_GPIO_REGISTERED && gpio_state.state[i].gpio_value == 0)
            return 1;
    }


    if (gpio_state.ave_power > DEVDRV_GPIO_AVE_POWER_ALERT) {
        return 1;
    }
    return 0;
}


STATIC void devdrv_lc_gpio_handle(unsigned long data)
{
    unsigned long flags;
    spin_lock_irqsave(&gpio_state.gpio_lock, flags);
    gpio_state.has_send_raise_freq_last_5ms = 0;

    if (devdrv_lc_is_in_over_current_status()) {
        spin_unlock_irqrestore(&gpio_state.gpio_lock, flags);

        if (gpio_state.aicore_freq_state != DEVDRV_ARCORE_FREQ_STATE_BOTTOM) {
            devdrv_inform_lpm3_change_aicore_freq(0, 1);
        }
    } else {
        spin_unlock_irqrestore(&gpio_state.gpio_lock, flags);

        if (gpio_state.aicore_freq_state != DEVDRV_ARCORE_FREQ_STATE_TOP) {
            devdrv_inform_lpm3_change_aicore_freq(1, 1);
        }
    }
}

STATIC void devdrv_lc_i2c_read_handle(void)
{
    int power = 0;
    int ret;
    int i;
    int total_power = 0;
    unsigned long flags;

    if (devdrv_check_ina2xx() == 0) {
        return;
    }
    ret = ina2xx_get_value(INA2XX_POWER, &power);
    if (ret != 0 || power == 0) {
        return;
    }
    spin_lock_irqsave(&gpio_state.gpio_lock, flags);
    if (gpio_state.curr_index >= DEVDRV_GPIO_AVE_CYCLE_TIME)
        gpio_state.curr_index = 0;

    gpio_state.ave_power_arr[gpio_state.curr_index] = power;
    gpio_state.curr_index++;


    for (i = 0; i < DEVDRV_GPIO_AVE_CYCLE_TIME; i++)
        total_power += gpio_state.ave_power_arr[i];
    gpio_state.ave_power = total_power / DEVDRV_GPIO_AVE_CYCLE_TIME;

    if (devdrv_lc_is_in_over_current_status()) {
        spin_unlock_irqrestore(&gpio_state.gpio_lock, flags);

        if (gpio_state.aicore_freq_state != DEVDRV_ARCORE_FREQ_STATE_BOTTOM) {
            devdrv_inform_lpm3_change_aicore_freq(0, 1);
        }
    } else {
        spin_unlock_irqrestore(&gpio_state.gpio_lock, flags);
        if (gpio_state.aicore_freq_state != DEVDRV_ARCORE_FREQ_STATE_TOP) {
            devdrv_inform_lpm3_change_aicore_freq(1, 1);
        }
    }
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0)
STATIC void devdrv_lc_timer_handle(unsigned long data)
#else
STATIC void devdrv_lc_timer_handle(struct timer_list *t)
#endif
{
    static int count = 0;

    count++;
    if (count >= (DEVDRV_GPIO_CYCLE_MS / DEVDRV_I2C_READ_MS)) {
        devdrv_lc_gpio_handle(0);
        count = 0;
    }
    if (devdrv_check_ina2xx() != 0) {
        gpio_state.task_status = TASK_RUN;
        wake_up(&gpio_state.i2c_read_wait);
    }
    gpio_state.timer.expires = jiffies + msecs_to_jiffies(DEVDRV_I2C_READ_MS);
    add_timer(&gpio_state.timer);
}

STATIC irqreturn_t devdrv_lc_gpio_irq_handle(int index)
{
    int gpio_value, gpio;
    unsigned long flags;

    if (index >= DEVDRV_GPIO_LC_CNT || index < 0) {
        devdrv_drv_err("invalid index, index = %d\n", index);
        return IRQ_NONE;
    }

    gpio = gpio_state.state[index].gpio;
    if (!gpio_is_valid(gpio)) {
        devdrv_drv_err("invalid gpio, gpio = %d\n", gpio);
        return IRQ_NONE;
    }

    gpio_value = gpio_get_value(gpio);

    gpio_state.state[index].gpio_value = gpio_value;

    devdrv_drv_warn("gpio=%d, value=%d\n", gpio, gpio_value);


    if (gpio_value <= 0) {
        spin_lock_irqsave(&gpio_state.gpio_lock, flags);

        if (gpio_state.has_send_raise_freq_last_5ms) {
            spin_unlock_irqrestore(&gpio_state.gpio_lock, flags);
        } else {
            gpio_state.has_send_raise_freq_last_5ms = 1;
            devdrv_inform_lpm3_change_aicore_freq(gpio_value, 0);
            spin_unlock_irqrestore(&gpio_state.gpio_lock, flags);
        }
    }

    return IRQ_HANDLED;
}

STATIC irqreturn_t devdrv_lc_gpio72_irq(int irq, void *data)
{
    return devdrv_lc_gpio_irq_handle(0);
}

STATIC irqreturn_t devdrv_lc_gpio1_irq(int irq, void *data)
{
    return devdrv_lc_gpio_irq_handle(1);
}

STATIC irqreturn_t devdrv_lc_gpio73_irq(int irq, void *data)
{
    int gpio_value, gpio;

    if (gpio_state.state[2].gpio_reg_flag != DEVDRV_GPIO_REGISTERED_NO_NEED_HANDLE) {
        devdrv_drv_err("invalid gpio_reg_flag, gpio_reg_flag=%d\n", gpio_state.state[2].gpio_reg_flag);
        return IRQ_NONE;
    }

    gpio = gpio_state.state[2].gpio;
    if (!gpio_is_valid(gpio)) {
        devdrv_drv_err("invalid gpio, gpio=%d\n", gpio);
        return IRQ_NONE;
    }

    gpio_value = gpio_get_value(gpio);

    devdrv_drv_warn("recvd gpio1 limit current irq, gpio=%d, value=%d\n", gpio, gpio_value);

    return IRQ_HANDLED;
}

STATIC void devdrv_lc_init_gpio_state(void)
{
    unsigned long flags;
    spin_lock_init(&gpio_state.gpio_lock);

    spin_lock_irqsave(&gpio_state.gpio_lock, flags);
    gpio_state.has_send_raise_freq_last_5ms = 0;

    gpio_state.aicore_freq_state = DEVDRV_ARCORE_FREQ_STATE_TOP;

    gpio_state.state[0].gpio = -1;
    gpio_state.state[0].gpio_reg_flag = DEVDRV_GPIO_UNREGISTERED;
    gpio_state.state[0].gpio_name = "lc-gpio72";
    gpio_state.state[0].gpio_label = "lc-gpio72_label";
    gpio_state.state[0].gpio_irqname = "lc-gpio72_irq";
    gpio_state.state[0].gpio_irq_handle = devdrv_lc_gpio72_irq;
    gpio_state.state[0].gpio_value = 1;

    gpio_state.state[1].gpio = -1;
    gpio_state.state[1].gpio_reg_flag = DEVDRV_GPIO_UNREGISTERED;
    gpio_state.state[1].gpio_name = "lc-gpio1";
    gpio_state.state[1].gpio_label = "lc-gpio1_label";
    gpio_state.state[1].gpio_irqname = "lc-gpio1_irq";
    gpio_state.state[1].gpio_irq_handle = devdrv_lc_gpio1_irq;
    gpio_state.state[1].gpio_value = 1;

    gpio_state.state[2].gpio = -1;
    gpio_state.state[2].gpio_reg_flag = DEVDRV_GPIO_UNREGISTERED;
    gpio_state.state[2].gpio_name = "lc-gpio73";
    gpio_state.state[2].gpio_label = "lc-gpio73_label";
    gpio_state.state[2].gpio_irqname = "lc-gpio73_irq";
    gpio_state.state[2].gpio_irq_handle = devdrv_lc_gpio73_irq;
    gpio_state.state[2].gpio_value = 1;

    spin_unlock_irqrestore(&gpio_state.gpio_lock, flags);
}

STATIC void devdrv_lc_uninit_gpio_state(void)
{
    gpio_state.state[0].gpio = -1;
    gpio_state.state[0].gpio_reg_flag = DEVDRV_GPIO_UNREGISTERED;

    gpio_state.state[1].gpio = -1;
    gpio_state.state[1].gpio_reg_flag = DEVDRV_GPIO_UNREGISTERED;

    gpio_state.state[2].gpio = -1;
    gpio_state.state[2].gpio_reg_flag = DEVDRV_GPIO_UNREGISTERED;
}

STATIC void devdrv_stop_calc_ave_current(void)
{
    gpio_state.task_status = TASK_STOP;
    ina2xx_unregister_drv();
}
int devdrv_in2xx_task_handle(void *data)
{
    while (!kthread_should_stop()) {
        wait_event(gpio_state.i2c_read_wait, gpio_state.task_status == TASK_RUN);
        devdrv_lc_i2c_read_handle();
        gpio_state.task_status = TASK_STOP;
    }
    return 0;
}
STATIC void devdrv_calc_ave_current(void)
{
    struct task_struct *ina2xx_task = NULL;
    struct sched_param param;
    int policy;

    init_waitqueue_head(&gpio_state.i2c_read_wait);
    gpio_state.task_status = TASK_STOP;
    ina2xx_register_drv();
    devdrv_drv_info("Board id is %d\n", devdrv_get_boardid());
    if (devdrv_check_ina2xx() != 0) {
        devdrv_drv_info("Use Ina231 to get current info\n");
        ina2xx_task = kthread_create(devdrv_in2xx_task_handle, NULL, "ina2xx_task");
        if (IS_ERR(ina2xx_task)) {
            devdrv_drv_err("create kthread ina2xx_task failed!\n");
            return;
        }
        gpio_state.ina2xx_task = ina2xx_task;
        /* set the thread's priority to 80, the bigger sched_priority, the higher priority */
        param.sched_priority = (MAX_RT_PRIO - 20);
        policy = SCHED_RR;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
        (void)sched_setscheduler(ina2xx_task, policy, &param);
#else
        sched_set_fifo(ina2xx_task);
#endif
        wake_up_process(ina2xx_task);
    }
}

void devdrv_lc_gpioirq_unregister(void)
{
    struct devdrv_info *dev_info = NULL;
    int i, gpio;

    dev_info = devdrv_get_default_devdrv_info();
    if (dev_info == NULL) {
        devdrv_drv_err("get default dev_info failed\n");
        return;
    }

    // gpio73
    if (gpio_state.state[0].gpio >= 0) {
        timer_shutdown_sync(&gpio_state.state[0].timer);
    }

    // gpio1
    if (gpio_state.state[1].gpio >= 0) {
        timer_shutdown_sync(&gpio_state.state[1].timer);
    }

    devdrv_stop_calc_ave_current();

    for (i = 0; i < DEVDRV_GPIO_LC_CNT; i++) {
        gpio = gpio_state.state[i].gpio;
        devdrv_drv_info("lc(%d), gpio(%d), gpio_reg_flag(%d).\n", i, gpio, gpio_state.state[i].gpio_reg_flag);
        if ((gpio_state.state[i].gpio_reg_flag == DEVDRV_GPIO_REGISTERED ||
            gpio_state.state[i].gpio_reg_flag == DEVDRV_GPIO_REGISTERED_NO_NEED_HANDLE) &&
            gpio_is_valid(gpio)) {
            devdrv_drv_info("gpio is valid, %d", gpio);
            devm_gpio_free(dev_info->dev, gpio);
            devm_free_irq(dev_info->dev, gpio_to_irq(gpio), dev_info);
        }

        devdrv_drv_info("devdrv_gpioirq_free finish,"
            " gpio = %d, gpio_reg_flag=%d\n",
            gpio, gpio_state.state[i].gpio_reg_flag);
    }

    devdrv_lc_uninit_gpio_state();
}

STATIC int devdrv_lc_gpioirq_request(struct devdrv_info *dev_info, int index)
{
    const char *gpio_label = NULL, *gpio_irqname = NULL;
    int ret = 0;
    int gpio;
    int i;

    if (index >= DEVDRV_GPIO_LC_CNT || index < 0) {
        devdrv_drv_err("index = %d, overflow...\n", index);
        return -EINVAL;
    }
    /* inital ave power */
    gpio_state.curr_index = 0;
    gpio_state.ave_power = 0;
    for (i = 0; i < DEVDRV_GPIO_AVE_CYCLE_TIME; i++)
        gpio_state.ave_power_arr[i] = 0;
    gpio = gpio_state.state[index].gpio;
    gpio_label = gpio_state.state[index].gpio_label;
    gpio_irqname = gpio_state.state[index].gpio_irqname;

    if (!gpio_is_valid(gpio)) {
        devdrv_drv_err("invalid gpio, gpio = %d\n", gpio);
        return -EINVAL;
    }

    if (devm_gpio_request_one(dev_info->dev, gpio, GPIOF_IN, gpio_label)) {
        devdrv_drv_err("gpio request failed, %s\n", gpio_label);
        return -EINVAL;
    } else {
        ret = devm_request_irq(dev_info->dev, gpio_to_irq(gpio), gpio_state.state[index].gpio_irq_handle,
            IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, gpio_irqname, dev_info);

        if (ret) {
            devdrv_drv_err("request irq failed, ret = %d, "
                "gpio_irqname=%s\n",
                ret, gpio_irqname);
            devm_gpio_free(dev_info->dev, gpio);
            return ret;
        }
    }

    return ret;
}

void devdrv_m3_notice_aicore_freq_state(int cmd)
{
    unsigned long flags;
    devdrv_drv_info("recvd cmd=%d\n", cmd);
    if (cmd == 0) {
        spin_lock_irqsave(&gpio_state.gpio_lock, flags);
        gpio_state.aicore_freq_state = DEVDRV_ARCORE_FREQ_STATE_BOTTOM;
        spin_unlock_irqrestore(&gpio_state.gpio_lock, flags);
    } else if (cmd == 1) {
        spin_lock_irqsave(&gpio_state.gpio_lock, flags);
        gpio_state.aicore_freq_state = DEVDRV_ARCORE_FREQ_STATE_TOP;
        spin_unlock_irqrestore(&gpio_state.gpio_lock, flags);
    } else if (cmd == 2) {
        spin_lock_irqsave(&gpio_state.gpio_lock, flags);
        gpio_state.aicore_freq_state = DEVDRV_ARCORE_FREQ_STATE_NORMAL;
        spin_unlock_irqrestore(&gpio_state.gpio_lock, flags);
    } else {
        devdrv_drv_err("cmd(%d) is err.\n", cmd);
    }
}

int devdrv_lc_gpioirq_register(void)
{
    struct devdrv_info *dev_info = NULL;
    struct device_node *np = NULL;
    int ret, i;

    devdrv_lc_init_gpio_state();

    dev_info = devdrv_get_default_devdrv_info();
    if (dev_info == NULL) {
        devdrv_drv_err("get default dev_info failed.\n");
        return -ENODEV;
    }

    np = dev_info->dev->of_node;

    for (i = 0; i < DEVDRV_GPIO_LC_CNT; i++)
        gpio_state.state[i].gpio = of_get_named_gpio(np, gpio_state.state[i].gpio_name, 0);

    devdrv_drv_info("limit current register, gpio(%d, %d, %d).\n", gpio_state.state[0].gpio, gpio_state.state[1].gpio,
        gpio_state.state[2].gpio);

    if (gpio_state.state[0].gpio < 0 && gpio_state.state[1].gpio < 0) {
        devdrv_drv_info("this board not support gpio for"
            " limit current, gpio(%d, %d, %d).\n",
            gpio_state.state[0].gpio, gpio_state.state[1].gpio, gpio_state.state[2].gpio);
        return 0;
    }

    for (i = 0; i < DEVDRV_GPIO_LC_CNT; i++) {
        if (gpio_state.state[i].gpio >= 0) {
            ret = devdrv_lc_gpioirq_request(dev_info, i);
            if (ret) {
                devdrv_drv_err("lc(%d), gpio(%d), registered(%d), "
                    "request failed(%d).\n",
                    i, gpio_state.state[i].gpio, gpio_state.state[i].gpio_reg_flag, ret);

                devdrv_lc_gpioirq_unregister();
                return ret;
            }

            gpio_state.state[i].gpio_reg_flag = DEVDRV_GPIO_REGISTERED;

            devdrv_drv_info("index(%d), gpio(%d), registered(%d), "
                "request succ.\n",
                i, gpio_state.state[i].gpio, gpio_state.state[i].gpio_reg_flag);
        }
    }

    devdrv_calc_ave_current();

    if (gpio_state.state[0].gpio >= 0 || gpio_state.state[1].gpio >= 0) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0)
        setup_timer(&gpio_state.timer, devdrv_lc_timer_handle, 0);
#else
        timer_setup(&gpio_state.timer, devdrv_lc_timer_handle, 0);
#endif

        gpio_state.timer.expires = jiffies + msecs_to_jiffies(DEVDRV_I2C_READ_MS);
        add_timer(&gpio_state.timer);
    }


    // gpio72
    if (gpio_state.state[2].gpio >= 0)
        gpio_state.state[2].gpio_reg_flag = DEVDRV_GPIO_REGISTERED_NO_NEED_HANDLE;

    return 0;
}
