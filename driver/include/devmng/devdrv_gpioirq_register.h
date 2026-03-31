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


#ifndef __DEVDRV_GPIOIRQ_REGISTER_H
#define __DEVDRV_GPIOIRQ_REGISTER_H
#include <linux/interrupt.h>
#include <linux/timer.h>

#define DEVDRV_GPIO_LC_CNT 3
#define DEVDRV_GPIO_TIMER_MS 100
#define DEVDRV_GPIO_VALUE 2


#define DEVDRV_ARCORE_FREQ_STATE_TOP 1
#define DEVDRV_ARCORE_FREQ_STATE_BOTTOM 2
#define DEVDRV_ARCORE_FREQ_STATE_NORMAL 3

#define DEVDRV_GPIO_REGISTERED 1
#define DEVDRV_GPIO_UNREGISTERED 2
#define DEVDRV_GPIO_REGISTERED_NO_NEED_HANDLE 3

#define DEVDRV_GPIO_CYCLE_MS 250
#define DEVDRV_I2C_READ_MS 5

#define DEVDRV_GPIO_AVE_CYCLE_MS 5
#define DEVDRV_GPIO_AVE_CYCLE_TIME 200
#define DEVDRV_GPIO_INIT_AVE_CURRENT 1.1
#define DEVDRV_GPIO_AVE_POWER_ALERT 66000 // 12v 5.5A mW

#define BOARDID_MINI_RC_1 1000
#define BOARDID_ATLAS_200 1004
#define BOARDID_MINI_RC_2 2000
#define BOARDID_MINI_RC_EMMC 3004
#define BOARDID_MINI_RC_FLASH 4004

struct devdrv_per_gpio_state {
    int gpio_reg_flag;
    int gpio;
    const char *gpio_name;
    const char *gpio_label;
    const char *gpio_irqname;
    irqreturn_t (*gpio_irq_handle)(int, void *);

    struct timer_list timer;
    int gpio_value;
};

struct devdrv_gpio_state {
    int has_send_raise_freq_last_5ms;
    spinlock_t gpio_lock;
    int aicore_freq_state;

    struct task_struct *ina2xx_task;
    wait_queue_head_t i2c_read_wait;
    int task_status;

    int curr_index;
    int ave_power;
    int ave_power_arr[DEVDRV_GPIO_AVE_CYCLE_TIME];

    struct timer_list timer;

    // 0 -gpio73;1-gpio1;2-gpio72;gpio72 is disable
    struct devdrv_per_gpio_state state[DEVDRV_GPIO_LC_CNT];
};

#endif /* __DEVDRV_GPIOIRQ_REGISTER_H */
