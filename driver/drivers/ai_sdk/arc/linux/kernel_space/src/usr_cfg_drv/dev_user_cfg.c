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
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/securec.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gfp.h>
#include "dev_user_cfg.h"
#include "user_cfg_management.h"
#include "user_cfg_public.h"
#include "user_cfg_interface.h"
#include "ascend_platform.h"

typedef struct _cpu_cfg {
    unsigned int ctrl_cpu_num;
    unsigned int data_cpu_num;
    unsigned int aicpu_num;
} cpu_cfg_t;

typedef struct st_devtype {
    unsigned int dev_major;
    unsigned int dev_minor;
    struct cdev cdev; /* Char device structure */
    struct class *dev_class;
    cpu_cfg_t cpu_cfg_info;
} CDEV_ST;

CDEV_ST *g_infra_user_cfg_dev = NULL;

#define REG_NOT_DEFINE          0
#define MAX_DEVICE_NUM          4
dev_cpu_nums_cfg_t g_cpu_nums_cfg[MAX_DEVICE_NUM] = {0};
unsigned char g_cpu_nums_cfg_flg = 0;

#define USER_CFG_DEV_CLASS "user_cfg_dev_class"
#define DEV_USER_CFG_NAME_KER "user_config"
#define USER_CFG_DEV_MAX_COUNT 1
#define USER_CFG_DEVNODE_MODE 0600

#define CHECK_PARA_SIZE_RETURN(size, para_size) do { \
    if ((size) != (para_size)) {                       \
        DEV_USER_CFG_ERR("The prarameter size is incorrect.\n"); \
        return UC_ERR_PARA;                              \
    }                                                  \
} while (0)

STATIC int dev_user_cfg_get_boot_cfg(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret;
    void __iomem *regs = NULL;
    unsigned int boot_cfg = 0;

    if (SC_PAD_INFO_BASE == REG_NOT_DEFINE) {
        boot_cfg = 0; // return CHIP_INFO_SOLO
        ret = copy_to_user((void *)(uintptr_t)arg, &boot_cfg, sizeof(int));
        if (ret) {
            DEV_USER_CFG_ERR("Copy boot configuration to user space failed. (ret=%d)\n", ret);
            return ret;
        }

        return 0;
    }

    regs = ioremap((unsigned long)SC_PAD_INFO_BASE, SC_PAD_INFO_PAGE_SIZE);
    if (regs == NULL) {
        DEV_USER_CFG_ERR("Remap page for boot cfg register failed.\n");
        ret = UC_ERR_MEM_ALLOC;
        return ret;
    }

    boot_cfg = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)regs + SC_PAD_INFO_OFFSET));
    (void)iounmap(regs);
    regs = NULL;

    boot_cfg &= BOOT_CFG_MASK;
    ret = copy_to_user((void *)(uintptr_t)arg, &boot_cfg, sizeof(int));
    if (ret) {
        DEV_USER_CFG_ERR("Copy boot configuration to user space failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}

#ifdef CFG_SOC_PLATFORM_MDC_V11
#  define SOC_CHIP_INFO_REG_BASE 0xC0140000UL
#  define SOC_CHIP_INFO_REG_OFFSET 0xFFF8
#  define CHIP_TYPE_REG_VALUE_MASK 0xFFFFF
#else
#  define SOC_CHIP_INFO_REG_BASE 0x8000F000UL
#  define SOC_CHIP_INFO_REG_OFFSET 0x0FF8
#  define CHIP_TYPE_REG_VALUE_MASK 0xF
#endif

__attribute__((unused)) STATIC int dev_user_cfg_get_sys_info(struct file *file, unsigned int cmd, unsigned long arg)
{
    void __iomem *reg_addr = NULL;
    u32 chip_type;
    u32 reg_value;
    int ret;

    reg_addr = ioremap(SOC_CHIP_INFO_REG_BASE + SOC_CHIP_INFO_REG_OFFSET, sizeof(u32));
    if (reg_addr == NULL) {
        DEV_USER_CFG_ERR("devm_ioremap failed.\n");
        return -EINVAL;
    }
    reg_value = readl(reg_addr);
    iounmap(reg_addr);
    reg_addr = NULL;

    chip_type = reg_value & CHIP_TYPE_REG_VALUE_MASK;
    DEV_USER_CFG_INFO("get chip_type success, chip_type = 0x%X.\n", chip_type);

    ret = copy_to_user((void *)(uintptr_t)arg, &chip_type, sizeof(u32));
    if (ret) {
        DEV_USER_CFG_ERR("Copy boot configuration to user space failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}

STATIC int dev_user_cfg_get_dts_cfg(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret;

    ret = copy_to_user((void *)(uintptr_t)arg, &g_infra_user_cfg_dev->cpu_cfg_info, sizeof(cpu_cfg_t));
    if (ret) {
        DEV_USER_CFG_ERR("Copy boot configuration to user space failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}

int dev_user_cfg_get_cpu_cfg(unsigned int dev_id, cpu_cfg_t *data, unsigned int size)
{
    int ret;
    uc_cpu_cfg_t flash_cpu_cfg = {0};

    if (size != sizeof(cpu_cfg_t)) {
        DEV_USER_CFG_ERR("Input data size out of range. (size=%u)\n", size);
        return -EINVAL;
    }

    ret = devdrv_config_get_cpu_cfg(dev_id, (unsigned char *)&flash_cpu_cfg, sizeof(uc_cpu_cfg_t));
    if (ret != 0) {
        DEV_USER_CFG_ERR("Call devdrv_config_get_cpu_cfg function fail. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    data->ctrl_cpu_num = flash_cpu_cfg.ctrl_cpu_num;
    data->data_cpu_num = flash_cpu_cfg.data_cpu_num;
    data->aicpu_num = flash_cpu_cfg.ai_cpu_num;
    return 0;
}
EXPORT_SYMBOL_GPL(dev_user_cfg_get_cpu_cfg);

/* define for CFG_FEATURE_CPU_NUMS_FIXED */
#define CPU_INFO_SIZE 64
#define DEV_CFG_DISABLE 0
#define DEV_CFG_ENABLE 1

STATIC int dev_user_cfg_parse_cpu_num(unsigned int dev_id, char buf[], u32 *cpu_num)
{
    int len;
    unsigned int i;
    char *start = NULL;
    char *end = NULL;

    start = buf;
    for (i = 0; i < dev_id; i++) {
        start = strchr(start, ',');
        if (start == NULL) {
            DEV_USER_CFG_ERR("String parse failed. (dev_id=%u; buf=\"%s\")\n", dev_id, buf);
            *cpu_num = 0;
            return 0; /* parse fail by format not support, buf return 0 to let the system start complete */
        }
        start++;
    }

    end = strchr(start, ',');
    if (end == NULL) {
        len = strlen(start);
    } else {
        len = strlen(start) - strlen(end);
        start[len] = '\0';
    }

    end = strchr(start, '-');
    if (end != NULL) {
        int start_num, end_num;
        len = strlen(start);
        end[0] = '\0';
        end++;
        start_num = (int)simple_strtol(start, NULL, 0);
        end_num = (int)simple_strtol(end, NULL, 0);
        *cpu_num = end_num - start_num + 1;
    } else {
        *cpu_num = 1;
    }

    return 0;
}

STATIC int dev_user_cfg_get_cpu_num(unsigned int dev_id, const char *cpu_type, u32 *cpu_num)
{
    struct file *file = NULL;
    char buf[CPU_INFO_SIZE];
    loff_t pos = 0;
    int len;

    file = filp_open(cpu_type, O_RDONLY, 0);
    if (IS_ERR(file)) {
        DEV_USER_CFG_INFO("Opening file is abnormal. (dev_id=%u; cpu_type=\"%s\")\n", dev_id, cpu_type);
        return -EINVAL;
    }

    len = kernel_read(file, buf, CPU_INFO_SIZE - 1, &pos);
    filp_close(file, NULL);
    file = NULL;

    if (len == 1) {
        /* file don't have content */
        DEV_USER_CFG_INFO("Reading file is abnormal. (dev_id=%u; cpu_type=\"%s\"; read_len=%d)\n",
            dev_id, cpu_type, len);
        *cpu_num = 0;
        return 0;
    } else if (len <= 0) {
        DEV_USER_CFG_ERR("File read failed. (dev_id=%u; cpu_type=\"%s\"; read_len=%d)\n", dev_id, cpu_type, len);
        return -EINVAL;
    }
    buf[len - 1] = '\0';

    DEV_USER_CFG_INFO("File read success. (dev_id=%u; cpu_group=\"%s\"; read_len=%d; buf=%s)\n",
        dev_id, cpu_type, len, buf);

    return dev_user_cfg_parse_cpu_num(dev_id, buf, cpu_num);
}

int dev_user_cfg_init_cpu_info(unsigned int dev_num)
{
    int cpu_num = 0;
    unsigned int dev_id;

    for (dev_id = 0; dev_id < dev_num; dev_id++) {
        int ret;

        DEV_USER_CFG_INFO("Get cpu information from cgroup config. (dev_id=%u)\n", dev_id);

        ret = dev_user_cfg_get_cpu_num(dev_id, "/sys/fs/cgroup/cpuset/CtrlCPU/cpuset.cpus", &cpu_num);
        if (ret) {
            DEV_USER_CFG_ERR("Ctrl cpu information got failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
            return ret;
        }
        g_cpu_nums_cfg[dev_id].ccpu_num = cpu_num;
        g_cpu_nums_cfg[dev_id].ccpu_os_sched = DEV_CFG_ENABLE;
        DEV_USER_CFG_INFO("Get the cpu_num. (dev_id=%u; cpu_num=%d)\n", dev_id, cpu_num);

        ret = dev_user_cfg_get_cpu_num(dev_id, "/sys/fs/cgroup/cpuset/DataCPU/cpuset.cpus", &cpu_num);
        if (ret) {
            DEV_USER_CFG_ERR("Data cpu information got failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
            return ret;
        }
        g_cpu_nums_cfg[dev_id].dcpu_num = cpu_num;
        g_cpu_nums_cfg[dev_id].dcpu_os_sched = DEV_CFG_ENABLE;
        DEV_USER_CFG_INFO("Get the cpu_num. (dev_id=%u; cpu_num=%d)\n", dev_id, cpu_num);

        ret = dev_user_cfg_get_cpu_num(dev_id, "/sys/fs/cgroup/cpuset/AICPU/cpuset.cpus", &cpu_num);
        if (ret) {
            DEV_USER_CFG_ERR("AI cpu information got failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
            return ret;
        }
        g_cpu_nums_cfg[dev_id].aicpu_num = cpu_num;
        g_cpu_nums_cfg[dev_id].aicpu_os_sched = DEV_CFG_ENABLE;
        DEV_USER_CFG_INFO("Get the cpu_num. (dev_id=%u; cpu_num=%d)\n", dev_id, cpu_num);
    }

    /* stub: get ts cpu info */
    g_cpu_nums_cfg[dev_id].tscpu_num = 0;
    g_cpu_nums_cfg[dev_id].tscpu_os_sched = DEV_CFG_DISABLE;

    return 0;
}

int dev_user_cfg_get_cpu_number(u32 dev_id, dev_cpu_nums_cfg_t *cpu_nums_cfg)
{
    static unsigned char log_flg = 0;
    unsigned int dev_num = 0;
    int ret;

    if (cpu_nums_cfg == NULL) {
        DEV_USER_CFG_ERR("The cpu_nums_cfg is NULL.\n");
        return -EINVAL;
    }

    ret = devdrv_config_get_dev_num(&dev_num);
    if (ret || dev_id >= dev_num) {
        DEV_USER_CFG_ERR("Get device number Failed or invalid parameter. (ret=%d; dev_id=%d; dev_num=%d)\n",
            ret, dev_id, dev_num);
        return ret;
    }

    if (g_cpu_nums_cfg_flg == 0) {
        ret = dev_user_cfg_init_cpu_info(dev_num);
        if (ret) {
            DEV_USER_CFG_ERR("Failed to invoke the dev_user_cfg_init_cpu_info. (ret=%d)\n", ret);
            return ret;
        }
        g_cpu_nums_cfg_flg = 1;
    }

    cpu_nums_cfg->aicpu_num = g_cpu_nums_cfg[dev_id].aicpu_num;
    cpu_nums_cfg->aicpu_os_sched = g_cpu_nums_cfg[dev_id].aicpu_os_sched;
    cpu_nums_cfg->tscpu_num = g_cpu_nums_cfg[dev_id].tscpu_num;
    cpu_nums_cfg->tscpu_os_sched = g_cpu_nums_cfg[dev_id].tscpu_os_sched;
    cpu_nums_cfg->ccpu_num = g_cpu_nums_cfg[dev_id].ccpu_num;
    cpu_nums_cfg->ccpu_os_sched = g_cpu_nums_cfg[dev_id].ccpu_os_sched;
    cpu_nums_cfg->dcpu_num = g_cpu_nums_cfg[dev_id].dcpu_num;
    cpu_nums_cfg->dcpu_os_sched = g_cpu_nums_cfg[dev_id].dcpu_os_sched;

    if (log_flg == 0) {
        log_flg = 1;
        DEV_USER_CFG_INFO("Cpus number config. (ccpu_num=%u; ccpu_os_sched=%u)\n",
            cpu_nums_cfg->ccpu_num, cpu_nums_cfg->ccpu_os_sched);
        DEV_USER_CFG_INFO("Cpus number config. (dcpu_num=%u; dcpu_os_sched=%u)\n",
            cpu_nums_cfg->dcpu_num, cpu_nums_cfg->dcpu_os_sched);
        DEV_USER_CFG_INFO("Cpus number config. (aicpu_num=%u; aicpu_os_sched=%u)\n",
            cpu_nums_cfg->aicpu_num, cpu_nums_cfg->aicpu_os_sched);
        DEV_USER_CFG_INFO("Cpus number config. (tscpu_num=%u; tscpu_os_sched=%u)\n",
            cpu_nums_cfg->tscpu_num, cpu_nums_cfg->tscpu_os_sched);
    }

    return 0;
}
EXPORT_SYMBOL(dev_user_cfg_get_cpu_number);

STATIC long dev_user_cfg_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    long ret;

    DRV_CHECK_RETV((file != NULL), -EINVAL);
    DRV_CHECK_RETV((arg != 0), -EINVAL);

    switch (cmd) {
        case USER_CFG_FLASH_OP:
            ret = (long)devdrv_config_ioctl(file, cmd, arg);
            break;
        case USER_CFG_GET_BOOT_CFG:
            ret = (long)dev_user_cfg_get_boot_cfg(file, cmd, arg);
            break;
        case USER_CFG_GET_DTS_CFG:
            ret = (long)dev_user_cfg_get_dts_cfg(file, cmd, arg);
            break;
#if (defined(CFG_SOC_PLATFORM_MDC_V51)) || (defined(CFG_SOC_PLATFORM_MDC_V11))
        case USER_CFG_GET_SYS_INFO:
            ret = (long)dev_user_cfg_get_sys_info(file, cmd, arg);
            break;
#endif
        default:
            ret = UC_ERR_PARA;
            break;
    }

    if (ret == UC_ERR_ITEM_NOT_SET) {
        DEV_USER_CFG_WARN("Can't not find match item. (ret=%ld)\n", ret);
    } else if (ret != UC_OK) {
        DEV_USER_CFG_ERR("Ioctl failed. (ret=%ld)\n", ret);
    }

    return ret;
}

STATIC int dev_user_cfg_open(struct inode *pnode, struct file *pfile)
{
    DRV_CHECK_RETV((pnode != NULL), -EINVAL);
    DRV_CHECK_RETV((pfile != NULL), -EINVAL);

    if (g_infra_user_cfg_dev != NULL) {
        pfile->private_data = g_infra_user_cfg_dev;
    }

    return UC_OK;
}

STATIC int dev_user_cfg_release(struct inode *pnode, struct file *pfile)
{
    DRV_CHECK_RETV((pnode != NULL), -EINVAL);
    DRV_CHECK_RETV((pfile != NULL), -EINVAL);
    return UC_OK;
}

STATIC void dev_user_cfg_cleanup_dev(CDEV_ST *pdev)
{
    dev_t dev_no;

    if ((pdev == NULL) || (pdev->dev_class == NULL)) {
        DEV_USER_CFG_ERR("The input parameter is NULL. (pdev=%pK)\n", pdev);
        return;
    }

    dev_no = MKDEV(pdev->dev_major, pdev->dev_minor);
    cdev_del(&pdev->cdev);
    device_destroy(pdev->dev_class, dev_no);
    class_destroy(pdev->dev_class);
    unregister_chrdev_region(dev_no, USER_CFG_DEV_MAX_COUNT);

    return;
}

/* set cdev node 0640 */
char *dev_user_cfg_devnode(const struct device *d, umode_t *mode)
{
    (void)d;
    if (mode != NULL) {
        DEV_USER_CFG_INFO("Set the value of devnode to 0600.\n");
        *mode = USER_CFG_DEVNODE_MODE;
    }
    return NULL;
}

STATIC int dev_user_cfg_init_dev(CDEV_ST *pdev, struct file_operations *pfoprs)
{
    dev_t dev_no;
    int ret;
    struct device *dev = NULL;
    struct class *dev_class = NULL;

    if ((pdev == NULL) || (pfoprs == NULL)) {
        DEV_USER_CFG_ERR("Input parameter is NULL. (pdev=%pK; pfoprs=%pK)\n", pdev, pfoprs);
        return UC_ERR_PARA;
    }

    pdev->dev_class = NULL;

    ret = alloc_chrdev_region(&dev_no, 0, USER_CFG_DEV_MAX_COUNT, DEV_USER_CFG_NAME_KER);
    if (ret < 0) {
        DEV_USER_CFG_ERR("Allocate character device regiio failed. (ret=%d)\n", ret);
        return ret;
    }

    dev_class = class_create(USER_CFG_DEV_CLASS);
    if (IS_ERR_OR_NULL(dev_class)) {
        DEV_USER_CFG_ERR("Create device class for user config error.\n");
        ret = PTR_ERR(dev_class);
        goto exit_unregister_chrdev_region;
    }

    dev_class->devnode = dev_user_cfg_devnode;

    dev = device_create(dev_class, NULL, dev_no, NULL, DEV_USER_CFG_NAME_KER);
    if (IS_ERR(dev)) {
        DEV_USER_CFG_ERR("Create device error.\n");
        ret = PTR_ERR(dev);
        goto exit_class_destroy;
    }

    cdev_init(&pdev->cdev, pfoprs);
    pdev->cdev.owner = THIS_MODULE;
    ret = cdev_add(&pdev->cdev, dev_no, 1);
    if (ret) {
        DEV_USER_CFG_ERR("Add character device failed. (ret=%d)\n", ret);
        goto exit_device_create;
    }

    pdev->dev_class = dev_class;
    pdev->dev_major = MAJOR(dev_no);
    pdev->dev_minor = MINOR(dev_no);

    return 0;

exit_device_create:
    device_destroy(dev_class, dev_no);
exit_class_destroy:
    class_destroy(dev_class);
exit_unregister_chrdev_region:
    unregister_chrdev_region(dev_no, USER_CFG_DEV_MAX_COUNT);

    return ret;
}

STATIC int dev_user_cfg_probe(struct platform_device *pdev)
{
    int ret;

    /* when dts don't have cpu cfg, set default number to 0, and not return fail  */
    DEV_USER_CFG_INFO("User config probe.\n");
    ret = of_property_read_u32(pdev->dev.of_node, "ctrl_cpu_num", &g_infra_user_cfg_dev->cpu_cfg_info.ctrl_cpu_num);
    if (ret != 0) {
        DEV_USER_CFG_WARN("Dts ctrl cpu num is not set. (ret=%d)\n", ret);
        g_infra_user_cfg_dev->cpu_cfg_info.ctrl_cpu_num = 0;
    }

    ret = of_property_read_u32(pdev->dev.of_node, "data_cpu_num", &g_infra_user_cfg_dev->cpu_cfg_info.data_cpu_num);
    if (ret != 0) {
        DEV_USER_CFG_WARN("Dts data cpu num is not set. (ret=%d)\n", ret);
        g_infra_user_cfg_dev->cpu_cfg_info.data_cpu_num = 0;
    }

    ret = of_property_read_u32(pdev->dev.of_node, "aicpu_num", &g_infra_user_cfg_dev->cpu_cfg_info.aicpu_num);
    if (ret != 0) {
        DEV_USER_CFG_WARN("Dts aicpu num is not set. (ret=%d)\n", ret);
        g_infra_user_cfg_dev->cpu_cfg_info.aicpu_num = 0;
    }

    DEV_USER_CFG_INFO("Dts ctrl cpu num. (num=%u)\n", g_infra_user_cfg_dev->cpu_cfg_info.ctrl_cpu_num);
    DEV_USER_CFG_INFO("Dts data cpu num. (num=%u)\n", g_infra_user_cfg_dev->cpu_cfg_info.data_cpu_num);
    DEV_USER_CFG_INFO("Dts dts aicpu num. (num=%u)\n", g_infra_user_cfg_dev->cpu_cfg_info.aicpu_num);
    return 0;
}

STATIC void dev_user_cfg_remove(struct platform_device *pdev)
{
    (void)pdev;
    DEV_USER_CFG_INFO("User config remove.\n");
}

STATIC const struct of_device_id g_user_cfg_of_match[] = {
    { .compatible = "hisi-user-cfg" },
    {}
};

MODULE_DEVICE_TABLE(of, g_user_cfg_of_match);

STATIC struct platform_driver g_user_cfg_platform_driver = {
    .probe = dev_user_cfg_probe,
    .remove = dev_user_cfg_remove,
    .driver = {
        .name = "hisi-user-cfg",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(g_user_cfg_of_match),
    },
};

struct file_operations g_infra_user_cfg_fops = {
    .owner = THIS_MODULE,
    .open = dev_user_cfg_open,
    .release = dev_user_cfg_release,
    .unlocked_ioctl = dev_user_cfg_ioctl,
};

STATIC int __init dev_user_cfg_module_init(void)
{
    int ret;

    g_infra_user_cfg_dev = (CDEV_ST *)kzalloc(sizeof(CDEV_ST), GFP_KERNEL | __GFP_ACCOUNT);
    if (g_infra_user_cfg_dev == NULL) {
        DEV_USER_CFG_ERR("Allocate memory for device structure failed.\n");
        return UC_ERR_MEM_ALLOC;
    }

    /* create character devices */
    ret = dev_user_cfg_init_dev(g_infra_user_cfg_dev, &g_infra_user_cfg_fops);
    if (ret != 0) {
        DEV_USER_CFG_ERR("Initialize character device failed. (ret=%d)\n", ret);
        goto free_dev;
    }

    ret = platform_driver_register(&g_user_cfg_platform_driver);
    if (ret != 0) {
        DEV_USER_CFG_ERR("Register platform driver failed. (ret=%d)\n", ret);
        goto uninit_dev;
    }

    ret = devdrv_config_pss_cfg_init(PKCS_SIGN_TYPE_OFF);
    if (ret) {
        DEV_USER_CFG_ERR("Failed to invoke the devdrv_config_set_pss_cfg. (ret=%d)\n", ret);
    }


    return UC_OK;
uninit_dev:
    dev_user_cfg_cleanup_dev(g_infra_user_cfg_dev);
free_dev:
    DEV_USER_CFG_KFREE(g_infra_user_cfg_dev);
    return ret;
}


STATIC void __exit dev_user_cfg_module_exit(void)
{
    platform_driver_unregister(&g_user_cfg_platform_driver);
    devdrv_config_pss_cfg_uninit();

    if (g_infra_user_cfg_dev != NULL) {
        dev_user_cfg_cleanup_dev(g_infra_user_cfg_dev);
        DEV_USER_CFG_KFREE(g_infra_user_cfg_dev);
    }

    DEV_USER_CFG_INFO("User cfg drv clean ok.\n");

    return;
}

module_init(dev_user_cfg_module_init);
module_exit(dev_user_cfg_module_exit);

MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("dev firmware user configure driver");
