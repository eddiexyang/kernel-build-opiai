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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/sysfs.h>
#include <linux/securec.h>
#include <linux/io.h>
#include <linux/of_gpio.h>
#include <linux/string.h>
#include <asm/processor.h>
#include <linux/atomic.h>
#include <linux/version.h>
#include <linux/kallsyms.h>
#include <linux/kthread.h>
#include <linux/acpi.h>
#include <linux/cpumask.h>

#include "ascend_ctl_dev.h"
#include "ascend_ctl_sysfs.h"

STATIC ascend_ctl_sysfs_t g_ascend_ctl_sysfs = {0};

STATIC const char *g_board_name[] = {
#ifdef CFG_SOC_PLATFORM_MDC_V51
    "hisilicon,ascend610-asic",
    "hisilicon,ascend610-evb",
    "hisilicon,ascend610-fpga",
#else
    "hisilicon,hi1910p-asic",
    "hisilicon,hi1910p-evb",
    "hisilicon,hi1910p-fpga",
    "hisilicon,hi1910B-evb",
#endif
    NULL
};

typedef struct _board_id_to_smp {
    unsigned int board_id;
    unsigned int smp_dev_num;
} board_id_to_smp_t;

STATIC board_id_to_smp_t g_smp_dev_array[] = {
#ifdef CFG_FEATURE_SMP_ENABLE
    {BOARD_ID_DC_2P_150, SMP_DEV_NUM_2P},
    {BOARD_ID_DC_2P_151, SMP_DEV_NUM_2P},
    {BOARD_ID_DC_2P_153, SMP_DEV_NUM_2P},
    {BOARD_ID_DC_2P_160, SMP_DEV_NUM_2P},
    {BOARD_ID_DC_2P_EVB_A, SMP_DEV_NUM_2P},
    {BOARD_ID_DC_2P_EVB_B, SMP_DEV_NUM_2P},
    {BOARD_ID_HELPER_2P_PG, SMP_DEV_NUM_2P},
#endif
};

void ascend_ctl_set_platform_device(struct platform_device *pdev)
{
    g_ascend_ctl_sysfs.dev = &pdev->dev;
}

void ascend_ctl_get_dts_cpu_cfg(struct platform_device *pdev)
{
    int ret;

    /* when dts don't have cpu cfg, set default number to 0, and not return fail  */
    ret = device_property_read_u32(&pdev->dev, "ctrl_cpu_num", &g_ascend_ctl_sysfs.dts_cpu_cfg.ctrl_cpu_num);
    if (ret != 0) {
        ASCEND_CTL_WARN("Dts ctrl cpu num is not set. (ret=%d)\n", ret);
        g_ascend_ctl_sysfs.dts_cpu_cfg.ctrl_cpu_num = 0;
    }

    ret = device_property_read_u32(&pdev->dev, "data_cpu_num", &g_ascend_ctl_sysfs.dts_cpu_cfg.data_cpu_num);
    if (ret != 0) {
        ASCEND_CTL_WARN("Dts data cpu num is not set, (ret=%d)\n", ret);
        g_ascend_ctl_sysfs.dts_cpu_cfg.data_cpu_num = 0;
    }

    ret = device_property_read_u32(&pdev->dev, "aicpu_num", &g_ascend_ctl_sysfs.dts_cpu_cfg.aicpu_num);
    if (ret != 0) {
        ASCEND_CTL_WARN("Dts aicpu num is not set, (ret=%d)\n", ret);
        g_ascend_ctl_sysfs.dts_cpu_cfg.aicpu_num = 0;
    }
}

void ascend_ctl_check_dts_cpu_cfg_valid(void)
{
    int total_cpu_count = 0;
    int dts_cpu_num_sum = 0;
    bool cfg_invalid_flag = false;
    if (g_ascend_ctl_sysfs.smp_dev_num == 0) {
        ASCEND_CTL_ERR("smp_dev_num is zero.\n");
        return;
    }
    total_cpu_count = num_online_cpus() / g_ascend_ctl_sysfs.smp_dev_num;
    ASCEND_CTL_INFO("Get current total cpu count, (total_cpu_cnt=%d, onlinecpu=%d, smp_dev_num=%u)\n", total_cpu_count,
        num_online_cpus(), g_ascend_ctl_sysfs.smp_dev_num);

    if (((u64)g_ascend_ctl_sysfs.dts_cpu_cfg.ctrl_cpu_num + (u64)g_ascend_ctl_sysfs.dts_cpu_cfg.data_cpu_num +
        (u64)g_ascend_ctl_sysfs.dts_cpu_cfg.aicpu_num) > __UINT32_MAX__) {
        ASCEND_CTL_ERR("dts_cpu_num_sum is out of range uint32.\n");
        cfg_invalid_flag = true;
    } else {
        dts_cpu_num_sum = g_ascend_ctl_sysfs.dts_cpu_cfg.ctrl_cpu_num + g_ascend_ctl_sysfs.dts_cpu_cfg.data_cpu_num +
            g_ascend_ctl_sysfs.dts_cpu_cfg.aicpu_num;
        ASCEND_CTL_INFO("Get current dts cpu number sum, (dts_cpu_num_sum=%d)\n", dts_cpu_num_sum);
    }

    if (dts_cpu_num_sum != total_cpu_count || cfg_invalid_flag) {
        ASCEND_CTL_WARN("Dts data enter 1-x logic\n");
        g_ascend_ctl_sysfs.dts_cpu_cfg.ctrl_cpu_num = 1;
        g_ascend_ctl_sysfs.dts_cpu_cfg.data_cpu_num = 0;
        g_ascend_ctl_sysfs.dts_cpu_cfg.aicpu_num = total_cpu_count - 1;
    }
}

STATIC ssize_t ascend_ctl_show_smp_dev_num(char *buf, unsigned int size)
{
    int fill_size;

    fill_size = snprintf_s(buf, size, size - 1, "smp_dev_num=%u\n", g_ascend_ctl_sysfs.smp_dev_num);
    if (fill_size <= 0) {
        ASCEND_CTL_ERR("Show smp device number failed. (fill_size=%d)\n", fill_size);
        return 0;
    }

    return fill_size;
}

void flash_cpu_cfg_clear(void)
{
    g_ascend_ctl_sysfs.flash_cpu_cfg.ctrl_cpu_num = 0;
    g_ascend_ctl_sysfs.flash_cpu_cfg.data_cpu_num = 0;
    g_ascend_ctl_sysfs.flash_cpu_cfg.aicpu_num = 0;
}

int ascend_ctl_get_flash_cpu_cfg_task(void *data)
{
    int ret;
    get_cpu_cfg_handle_t cpu_cfg_handle;
    unsigned int *dev_id = (unsigned int *)data;
    cpu_cfg_handle = (get_cpu_cfg_handle_t)(uintptr_t)__symbol_get(USER_CFG_GET_CPU_CFG_FUNC_NAME);
    if (cpu_cfg_handle == NULL) {
        ASCEND_CTL_ERR("kallsyms_lookup_name failed.\n");
        flash_cpu_cfg_clear();
        up(&g_ascend_ctl_sysfs.flash_cpu_cfg_sema);
        do_exit(0);
        return -EINVAL;
    }

    ret = cpu_cfg_handle(*dev_id, &g_ascend_ctl_sysfs.flash_cpu_cfg, sizeof(cpu_cfg_t));
    __symbol_put(USER_CFG_GET_CPU_CFG_FUNC_NAME);
    if (ret != 0) {
        ASCEND_CTL_ERR("Cpu_cfg_handler failed. (ret=%d)\n", ret);
        flash_cpu_cfg_clear();
        up(&g_ascend_ctl_sysfs.flash_cpu_cfg_sema);
        do_exit(0);
        return ret;
    }
    ASCEND_CTL_INFO("ascend_ctl_get_flash_cpu_cfg success.\n");
    up(&g_ascend_ctl_sysfs.flash_cpu_cfg_sema);
    do_exit(0);
    return ret;
}

#define GET_FLASH_CPU_CFG_WAIT_TIMEOUT 3000
STATIC ssize_t ascend_ctl_show_flash_cpu_cfg(unsigned int dev_id, char *buf, unsigned int size)
{
    int ret, i;
    int fill_size = 0;
    char *fill_pos = buf;
    unsigned int remain_size = size;
    unsigned int cpu_cfg_data[3] = {0}; /* 3 cpu type */
    char *cpu_cfg_name[3] = { /* 3 cpu type */
        "flash_ctrl_cpu_num=", "flash_data_cpu_num=", "flash_aicpu_num="
    };
    struct task_struct *task = NULL;

    task = kthread_create(ascend_ctl_get_flash_cpu_cfg_task, (void*)&dev_id, "get_flash_cpu_cfg");
    if (IS_ERR(task) || (task == NULL)) {
        ASCEND_CTL_ERR("can not get cpu cfg from flash, errno=%ld.\n", PTR_ERR(task));
        flash_cpu_cfg_clear();
    } else {
        (void)wake_up_process(task);
        ret = down_timeout(&g_ascend_ctl_sysfs.flash_cpu_cfg_sema, msecs_to_jiffies(GET_FLASH_CPU_CFG_WAIT_TIMEOUT));
        if (ret != 0) {
            ASCEND_CTL_ERR("wait respone time out, ret=%d(s). device_id=%d\n", ret, dev_id);
            flash_cpu_cfg_clear();
        }
    }

    cpu_cfg_data[0] = g_ascend_ctl_sysfs.flash_cpu_cfg.ctrl_cpu_num;
    cpu_cfg_data[1] = g_ascend_ctl_sysfs.flash_cpu_cfg.data_cpu_num;
    cpu_cfg_data[2] = g_ascend_ctl_sysfs.flash_cpu_cfg.aicpu_num; /* index 2 */

    for (i = 0; i < 3; i++) { /* 3 cpu type */
        remain_size -= fill_size;
        fill_pos += fill_size;
        fill_size = snprintf_s(fill_pos, remain_size, remain_size - 1, "%s%u\n", cpu_cfg_name[i], cpu_cfg_data[i]);
        if (fill_size <= 0) {
            ASCEND_CTL_ERR("show cpu cfg[%d] fail, fill_size=%d.\n", i, fill_size);
            return 0;
        }
    }

    remain_size -= fill_size;
    return size - remain_size;
}

STATIC ssize_t ascend_ctl_show_dts_cpu_cfg(char *buf, unsigned int size)
{
    int i;
    int fill_size = 0;
    char *fill_pos = buf;
    unsigned int remain_size = size;
    unsigned int cpu_cfg_data[3] = {0}; /* 3 cpu type */
    char *cpu_cfg_name[3] = { /* 3 cpu type */
        "dts_ctrl_cpu_num=", "dts_data_cpu_num=", "dts_aicpu_num="
    };

    cpu_cfg_data[0] = g_ascend_ctl_sysfs.dts_cpu_cfg.ctrl_cpu_num;
    cpu_cfg_data[1] = g_ascend_ctl_sysfs.dts_cpu_cfg.data_cpu_num;
    cpu_cfg_data[2] = g_ascend_ctl_sysfs.dts_cpu_cfg.aicpu_num; /* index 2 */

    for (i = 0; i < 3; i++) { /* 3 cpu type */
        remain_size -= fill_size;
        fill_pos += fill_size;
        fill_size = snprintf_s(fill_pos, remain_size, remain_size - 1, "%s%u\n", cpu_cfg_name[i], cpu_cfg_data[i]);
        if (fill_size <= 0) {
            ASCEND_CTL_ERR("show cpu cfg[%d] fail, fill_size=%d.\n", i, fill_size);
            return 0;
        }
    }

    remain_size -= fill_size;
    return size - remain_size;
}

STATIC ssize_t ascend_ctl_show_dev_info(unsigned int dev_id, char *buf)
{
    ssize_t show_size;
    char *cur_pos = buf;
    ssize_t total_size = 0;

    /* show smp number */
    show_size = ascend_ctl_show_smp_dev_num(cur_pos, PAGE_SIZE);
    cur_pos += show_size;
    total_size += show_size;

    /* show flash cpu cfg */
    show_size = ascend_ctl_show_flash_cpu_cfg(dev_id, cur_pos, PAGE_SIZE - total_size);
    cur_pos += show_size;
    total_size += show_size;

    /* show dts cpu cfg */
    show_size = ascend_ctl_show_dts_cpu_cfg(cur_pos, PAGE_SIZE - total_size);
    cur_pos += show_size;
    total_size += show_size;
    return total_size;
}

ssize_t ascend_ctl_show_dev0_info(struct device *dev, struct device_attribute *attr, char *buf)
{
    return(ascend_ctl_show_dev_info(0, buf)); /* device 0 */
}

ssize_t ascend_ctl_show_dev1_info(struct device *dev, struct device_attribute *attr, char *buf)
{
    return(ascend_ctl_show_dev_info(1, buf)); /* device 1 */
}

ssize_t ascend_ctl_show_dev2_info(struct device *dev, struct device_attribute *attr, char *buf)
{
    return(ascend_ctl_show_dev_info(2, buf)); /* device 2 */
}

ssize_t ascend_ctl_show_dev3_info(struct device *dev, struct device_attribute *attr, char *buf)
{
    return(ascend_ctl_show_dev_info(3, buf));  /* device 3 */
}

STATIC struct device_attribute g_dev_attr_dev_info[SMP_DEV_MAX] = {
    {
        .attr = {
            .name = "device_info",
            .mode = S_IRUSR | S_IRGRP | S_IROTH
        },
        .show    = ascend_ctl_show_dev0_info,
        .store    = NULL,
    },
    {
        .attr = {
            .name = "device_info",
            .mode = S_IRUSR | S_IRGRP | S_IROTH
        },
        .show    = ascend_ctl_show_dev1_info,
        .store    = NULL,
    },
    {
        .attr = {
            .name = "device_info",
            .mode = S_IRUSR | S_IRGRP | S_IROTH
        },
        .show    = ascend_ctl_show_dev2_info,
        .store    = NULL,
    },
    {
        .attr = {
            .name = "device_info",
            .mode = S_IRUSR | S_IRGRP | S_IROTH
        },
        .show    = ascend_ctl_show_dev3_info,
        .store    = NULL,
    }
};

STATIC struct attribute *g_dev0_info_attrs[] = {
    &g_dev_attr_dev_info[0].attr,  /* device 0 */
    NULL,
};

STATIC struct attribute *g_dev1_info_attrs[] = {
    &g_dev_attr_dev_info[1].attr,  /* device 1 */
    NULL,
};

STATIC struct attribute *g_dev2_info_attrs[] = {
    &g_dev_attr_dev_info[2].attr,  /* device 2 */
    NULL,
};

STATIC struct attribute *g_dev3_info_attrs[] = {
    &g_dev_attr_dev_info[3].attr,  /* device 3 */
    NULL,
};

STATIC const struct attribute_group g_davinci_sysfs_group[SMP_DEV_MAX] = {
    {
        .attrs = g_dev0_info_attrs,
        .name = "davinci0",
    },
    {
        .attrs = g_dev1_info_attrs,
        .name = "davinci1",
    },
    {
        .attrs = g_dev2_info_attrs,
        .name = "davinci2",
    },
    {
        .attrs = g_dev3_info_attrs,
        .name = "davinci3",
    }
};

int ascend_ctl_get_dev_num(unsigned int *dev_num)
{
    *dev_num = g_ascend_ctl_sysfs.smp_dev_num;
    return 0;
}
EXPORT_SYMBOL_GPL(ascend_ctl_get_dev_num);

int ascend_ctl_get_dts_cpu_cfg_to_user_cfg(unsigned int *ctrl_cpu_num, unsigned int *data_cpu_num,
    unsigned int *ai_cpu_num)
{
    if (ctrl_cpu_num == NULL || data_cpu_num == NULL || ai_cpu_num == NULL) {
        ASCEND_CTL_ERR("Invalid pointer. (ctrl_cpu_num_is_null=%d; data_cpu_num_is_null=%d; ai_cpu_num_is_null=%d)\n",
            (ctrl_cpu_num == NULL), (data_cpu_num == NULL), (ai_cpu_num == NULL));
        return -ENOENT;
    }
    *ctrl_cpu_num = g_ascend_ctl_sysfs.dts_cpu_cfg.ctrl_cpu_num;
    *data_cpu_num = g_ascend_ctl_sysfs.dts_cpu_cfg.data_cpu_num;
    *ai_cpu_num = g_ascend_ctl_sysfs.dts_cpu_cfg.aicpu_num;
    return 0;
}
EXPORT_SYMBOL_GPL(ascend_ctl_get_dts_cpu_cfg_to_user_cfg);

STATIC int ascend_ctl_get_dts_board_id(unsigned int *board_id)
{
    int ret;
    int i;
    unsigned int id_array[BOARD_ID_LEN] = {0};
    struct device_node *np = NULL;

    for (i = 0; (g_board_name[i] != NULL) && (np == NULL); i++) {
        np = of_find_compatible_node(NULL, NULL, g_board_name[i]);
    }

    if (np == NULL) {
        ASCEND_CTL_WARN("Can't find valid board_name node.\n");
        return -ENOENT;
    }

    ret = of_property_read_u32_array(np, "hisi,boardid", id_array, BOARD_ID_LEN);
    if (ret != 0) {
        ASCEND_CTL_WARN("Can't read boardid message. (ret:%d).\n", ret);
        return ret;
    }

    /* the value of board_id is the sum of id_array[0] * 1000, id_array[1] * 100, id_array[2] * 10 and id_array[3] */
    *board_id = id_array[0] * 1000 + id_array[1] * 100 + id_array[2] * 10 + id_array[3];
    return 0;
}

STATIC int ascend_ctl_get_acpi_board_id(unsigned int *board_id)
{
    int ret;
    unsigned int id_array[BOARD_ID_LEN] = {0};

    ret = device_property_read_u32_array(g_ascend_ctl_sysfs.dev, "hisi,boardid", id_array, BOARD_ID_LEN);
    if (ret) {
        ASCEND_CTL_WARN("Can't read boardid from acpi. (ret=%d).\n", ret);
        return ret;
    }

    /* the value of board_id is the sum of id_array[0] * 1000, id_array[1] * 100, id_array[2] * 10 and id_array[3] */
    *board_id = id_array[0] * 1000 + id_array[1] * 100 + id_array[2] * 10 + id_array[3];
    return 0;
}

STATIC int ascend_ctl_get_board_id(unsigned int *board_id)
{
    int ret;

    if (g_ascend_ctl_sysfs.dev != NULL) {
        if (ACPI_COMPANION(g_ascend_ctl_sysfs.dev)) {
            return ascend_ctl_get_acpi_board_id(board_id);
        } else if (g_ascend_ctl_sysfs.dev->of_node) {
            return ascend_ctl_get_dts_board_id(board_id);
        }
    }

    /* no probe process */
    ret = ascend_ctl_get_dts_board_id(board_id);
    if (ret) {
        ASCEND_CTL_WARN("Get boardid from dts failed. (ret=%d)\n", ret);
        return ret;
    }
    return 0;
}

STATIC int ascend_ctl_get_smp_info(void)
{
    int ret, i;
    unsigned int board_id = 0;

    ret = ascend_ctl_get_board_id(&board_id);
    if (ret != 0) {
        ASCEND_CTL_WARN("Failed to invoke ascend_ctl_get_board_id. (ret=%d)\n", ret);
        return ret;
    }

    g_ascend_ctl_sysfs.smp_dev_num = SMP_DEV_NUM_1P;
    for (i = 0; i < sizeof(g_smp_dev_array) / sizeof(board_id_to_smp_t); i++) {
        if (board_id == g_smp_dev_array[i].board_id) {
            g_ascend_ctl_sysfs.smp_dev_num = g_smp_dev_array[i].smp_dev_num;
            break;
        }
    }

    ASCEND_CTL_INFO("Smp device number is %u.\n", g_ascend_ctl_sysfs.smp_dev_num);

    return 0;
}

int ascend_ctl_sysfs_init(void)
{
    int ret;
    unsigned int i;
    unsigned int exit_dev_num;

    if (g_ascend_ctl_sysfs.ready) {
        return 0;
    }

    ret = ascend_ctl_get_smp_info();
    if (ret != 0) {
        if (g_ascend_ctl_sysfs.dev == NULL) {
            ASCEND_CTL_WARN("Can't get smp info, maybe driver not ready. (ret=%d).\n", ret);
            return 0;
        } else {
            ASCEND_CTL_ERR("Get smp info failed. (ret=%d).\n", ret);
            return ret;
        }
    }

    sema_init(&g_ascend_ctl_sysfs.flash_cpu_cfg_sema, 0);
    if (g_ascend_ctl_sysfs.smp_dev_num > SMP_DEV_MAX) {
        ASCEND_CTL_ERR("smp device number[%u] is out of range.\n", g_ascend_ctl_sysfs.smp_dev_num);
        return -EINVAL;
    }

    /*  davinci kobject creat */
    g_ascend_ctl_sysfs.kobj = kobject_create_and_add("davinci", NULL);
    if (g_ascend_ctl_sysfs.kobj == NULL) {
        ASCEND_CTL_ERR("kobject_create_and_add error.\n");
        return -EINVAL;
    }

    for (i = 0; i < g_ascend_ctl_sysfs.smp_dev_num; i++) {
        ret = sysfs_create_group(g_ascend_ctl_sysfs.kobj, &g_davinci_sysfs_group[i]);
        if (ret != 0) {
            ASCEND_CTL_ERR("sysfs create group failed, ret %d.\n", ret);
            exit_dev_num = i;
            goto creat_group_exit;
        }
    }

    g_ascend_ctl_sysfs.ready = true;
    return 0;

creat_group_exit:
    for (i = 0; i < exit_dev_num; i++) {
        sysfs_remove_group(g_ascend_ctl_sysfs.kobj, &g_davinci_sysfs_group[i]);
    }
    kobject_put(g_ascend_ctl_sysfs.kobj);
    g_ascend_ctl_sysfs.kobj = NULL;
    return ret;
}

void ascend_ctl_sysfs_exit(void)
{
    unsigned int i;

    for (i = 0; i < g_ascend_ctl_sysfs.smp_dev_num; i++) {
        sysfs_remove_group(g_ascend_ctl_sysfs.kobj, &g_davinci_sysfs_group[i]);
    }

    kobject_put(g_ascend_ctl_sysfs.kobj);
    g_ascend_ctl_sysfs.kobj = NULL;
    g_ascend_ctl_sysfs.dev = NULL;
    g_ascend_ctl_sysfs.ready = false;
}
