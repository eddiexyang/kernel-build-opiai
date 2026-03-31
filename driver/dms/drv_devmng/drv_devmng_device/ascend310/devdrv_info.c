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


#include <linux/fs.h>
#include <linux/of_gpio.h>
#include <securec.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#ifdef CFG_SOC_PLATFORM_MINI
#include <linux/hisi-spmi.h>
#include <hisi_pmic.h>
#endif

#include "devdrv_manager_common.h"
#include "devdrv_manager.h"
#include "devdrv_ipc.h"
#include "icm_interface.h"
#include "tsmng_interface.h"

#ifdef CFG_BUILD_DEBUG
#include "devdrv_info_debug.h"
#endif

#define FLASH_MAX_ID 1000

#define FATAL_ERROR_MASK 0x4
#define IMPORTANT_ERROR_MASK 0x3
#define NORMAL_ERROR_MASK 0x2
#define TOTAL_ERROR_MASK 0x7
#define NONE_ERROR 0x0
#define ERROR_BIT_POSITION 25
#define ERROR_MAX_VALUE 0x4

/* soc die id */
#define SOC_DIEID_MAP_SIZE (4096UL * 2)
#define SYS_CTRL_BASE_ADDR 0x01100CE000
#define SYS_CTRL_SC_DIE_ID0_OFFSET 0x200
#define SYS_CTRL_SC_DIE_ID_REG_OFFSET 0x4

/* reset i2c controller */
#define SM_BUS_CTL_BASE_ADDR 0x130070000
#define RESET_I2C_OFFSET 0xd00
#define RESET_REG_VALUE_OFFSET 0x5d00
#define UNRESET_I2C_OFFSET 0xd04
#define I2C_RESET_MAP_SIZE (4096 * 6)

/* gpio retry read */
#define MAX_RETRY 0xffff
#define DEVDRV_GPIO_NAME "gpio-read"

/* xloader boot info */
#define SIZE_OF_64K 0x10000
#define SYSCTRL_REG_BASE 0x1100C0000
#define SC_SOFT_POR_RSV3 0xFF2C
#define SC_BAK_DATA14 0x3448
#define ERRNO_NONSUPPORT_ITEM 0x4

typedef enum {
    BOOT_FROM_ONCHIPROM,
    BOOT_FROM_XLOADER0,
    BOOT_FROM_XLOADER1,
    BOOT_FROM_BOTTOM
} BOOT_MODE;

#define DEV_UPDATE_XLOADER_AREA0 0x0
#define DEV_UPDATE_XLOADER_AREA1 0x1
#define DEV_UPDATE_XLOADER_BOTH 0x2

#define DEV_GET_CURR_BOOT_AREA 0
#define DEV_CLEAR_BOOT_COUNT 1

struct dmanage_flash_info {
    unsigned long flash_id;         /* combined device & manufacturer code */
    unsigned short device_id;       /* device id */
    unsigned short vendor;          /* the primary vendor id */
    unsigned int state;             /* flash health */
    unsigned long size;             /* total size in bytes */
    unsigned int sector_count;      /* number of erase units */
    unsigned short manufacturer_id; /* manufacturer id */
};

struct dmanage_temp_share_mem {
    s8 cluster_temp;
    s8 peri_temp;
    s8 aicore0_temp;
    s8 aicore1_temp;
    u8 aicore_limit;
    u8 aicore_total_period;
    u8 aicore_elim_period;
    u8 aicore_base_freq_l;
    u8 aicore_base_freq_h; /* default little-edian */
    s8 soc_max_temp;
    u8 ddr_capacity_l;
    u8 ddr_capacity_h;
    u8 ddr_freq_l;
    u8 ddr_freq_h;
    s8 limited_temp;
    s8 reset_temp;
};

int bbox_get_device_errorcode(u32 dev_id, u32 *e_code, u32 e_capacity);


STATIC int devdrv_get_pcie_id(unsigned long arg)
{
    struct dmanage_pcie_id_info pcie_id_info = { 0 };
    unsigned int dev_id;
    int ret;
    ret = copy_from_user_safe(&pcie_id_info, (void *)((uintptr_t)arg), sizeof(struct dmanage_pcie_id_info));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return -1;
    }

    dev_id = pcie_id_info.davinci_id;
    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("invalid device id, id = %d.\n", dev_id);
        return -1;
    }

    ret = devdrv_get_pcie_id_info(dev_id, &pcie_id_info);
    if (ret) {
        devdrv_drv_err("devdrv_get_pcie_id failed.\n");
        return -1;
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &pcie_id_info, sizeof(struct dmanage_pcie_id_info));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -1;
    }
    return 0;
}

STATIC int devdrv_get_flash_count(unsigned long arg)
{
    int count;
    int ret;

    count = 0;

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &count, sizeof(int));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -1;
    }
    return 0;
}

STATIC int devdrv_get_flash_info(unsigned long arg)
{
    struct dmanage_flash_info flash_info = {0};
    int ret;

    ret = copy_from_user_safe(&flash_info, (void *)((uintptr_t)arg), sizeof(struct dmanage_flash_info));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return -1;
    }

    if (flash_info.flash_id > FLASH_MAX_ID) {
        devdrv_drv_err("invalid flash index.\n");
        return -1;
    }

    flash_info.flash_id = 1;
    flash_info.device_id = 1;
    flash_info.vendor = 1;
    flash_info.state = 1;
    flash_info.size = 1;
    flash_info.sector_count = 1;
    flash_info.manufacturer_id = 1;

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &flash_info, sizeof(struct dmanage_flash_info));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -1;
    }
    return 0;
}

int devdrv_get_max_health_code(unsigned int dev_id, unsigned int *max_health_value,
    struct devdrv_error_code_para *error_code_para)
{
    int i = 0;
    u32 max = 0;
    u32 max_index = 0;
    u32 error_level = 0;
    u32 *err_code = NULL;

    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (max_health_value == NULL) ||
        (error_code_para == NULL)) {
        devdrv_drv_err("invalid device id, id = %d.\n", dev_id);
        return -EINVAL;
    }
    err_code = error_code_para->error_code;

    error_code_para->error_code_count = bbox_get_device_errorcode(dev_id, err_code, DMANAGE_ERROR_ARRAY_NUM);
    if (error_code_para->error_code_count < 0) {
        devdrv_drv_err("bbox_get_device_errorcode failed, ret = %d.\n", error_code_para->error_code_count);
        return -EINVAL;
    }

    if (error_code_para->error_code_count > DMANAGE_ERROR_ARRAY_NUM) {
        devdrv_drv_err("error_code_count(%d) is illegal.\n", error_code_para->error_code_count);
        return -EINVAL;
    }

    for (i = 0; i < error_code_para->error_code_count; i++) {
        error_level = ((error_code_para->error_code[i] >> ERROR_BIT_POSITION) & (TOTAL_ERROR_MASK));
        if (max < error_level) {
            max = error_level;
            max_index = i;
        }
    }

    max = (max > ERROR_MAX_VALUE) ? ERROR_MAX_VALUE : max;

    *max_health_value = ((max > 0) ? (max - 1) : 0);

    return 0;
}

void devdrv_fresh_error_code_to_shm(struct work_struct *work)
{
    struct devdrv_error_code_para para = { 0, {0}, 0 };
    struct devdrv_info *dev_info = NULL;
    unsigned int health_code;
    int ret, i;

    dev_info = container_of(work, struct devdrv_info, work);
    if ((dev_info == NULL) || (dev_info->shm_status == NULL)) {
        devdrv_drv_err("dev_info=NULL or dev_info->shm_status=NULL.\n");
        return;
    }

    ret = devdrv_get_max_health_code(dev_info->dev_id, &health_code, &para);
    if (ret) {
        devdrv_drv_err("[devid=%u] get health code fail, ret=%d.\n", dev_info->dev_id, ret);
        return;
    }

    dev_info->shm_status->health_status = (u16)health_code;
    dev_info->shm_status->error_cnt = para.error_code_count;
    for (i = 0; i < DEVMNG_SHM_INFO_ERROR_CODE_LEN; i++) {
        dev_info->shm_status->error_code[i] = para.error_code[i];
    }
}

int devdrv_fresh_event_code_to_shm(u32 devid, u32 *health_code, u32 health_len,
    struct shm_event_code *event_code, u32 event_len)
{
    struct devdrv_info *dev_info = NULL;
    int event_cnt = 0;
    int i;

    dev_info = devdrv_manager_get_devdrv_info(devid);
    if ((dev_info == NULL) || (dev_info->shm_status == NULL)) {
        devdrv_drv_err("dev_info=NULL or dev_info->shm_status=NULL.\n");
        return -EINVAL;;
    }

    for (i = 0; i < VMNG_VDEV_MAX_PER_PDEV; i++) {
        dev_info->shm_status->dms_health_status[i] = (u16)health_code[i];
    }
    for (i = 0; i < DEVMNG_SHM_INFO_EVENT_CODE_LEN; i++) {
        if (event_code[i].event_code == 0) {
            break;
        }
        dev_info->shm_status->event_code[i].event_code = event_code[i].event_code;
        dev_info->shm_status->event_code[i].fid = event_code[i].fid;
        event_cnt++;
    }
    dev_info->shm_status->event_cnt = event_cnt;

    return 0;
}

STATIC int devdrv_get_health_code(unsigned long arg)
{
    struct devdrv_error_code_para para = { 0, { 0 }, 0 };
    struct ioctl_arg user_arg = { 0 };
    int ret;

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    ret = devdrv_get_max_health_code(user_arg.dev_id, &user_arg.data1, &para);
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return ret;
    }
    return 0;
}

STATIC int devdrv_get_error_code(unsigned long arg)
{
    struct devdrv_error_code_para error_code_para = { 0, { 0 }, 0 };
    u32 *err_code = error_code_para.error_code;
    struct ioctl_arg user_arg = { 0 };
    int ret;

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return -1;
    }

    if (user_arg.dev_id >= DEVDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("invalid device id, id = %d.\n", user_arg.dev_id);
        return -1;
    }

    error_code_para.error_code_count = bbox_get_device_errorcode(user_arg.dev_id, err_code, DMANAGE_ERROR_ARRAY_NUM);
    if (error_code_para.error_code_count < 0) {
        devdrv_drv_err("bbox_get_device_errorcode failed, ret = %d.\n", error_code_para.error_code_count);
        return -1;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &error_code_para, sizeof(struct devdrv_error_code_para));
    if (ret != 0) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -1;
    }

    return 0;
}

STATIC int devdrv_get_ddr_capacity(unsigned long arg)
{
    struct dmanage_temp_share_mem *share_mem_base = NULL;
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    struct ioctl_arg user_arg = { 0 };
    u32 capacity;
    u8 high;
    u8 low;
    int ret;

    dev_info = devdrv_get_default_devdrv_info();
    if (dev_info == NULL) {
        devdrv_drv_err("default dev_info is null, unable to get temperature.\n");
        return -1;
    }

    pdata = (struct devdrv_platform_data *)dev_info->pdata;
    share_mem_base = (struct dmanage_temp_share_mem *)pdata->ts_pdata[0].tsensor_shm_vaddr;

    high = share_mem_base->ddr_capacity_h;
    low = share_mem_base->ddr_capacity_l;
    capacity = (high << 8) | low;
    user_arg.data1 = capacity;

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -1;
    }

    return 0;
}

STATIC int devdrv_config_ecc_enable(unsigned long arg)
{
    int ret;
    struct ioctl_arg user_arg = { 0 };

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -1;
    }
    return 0;
}

int bbox_get_device_ecode_info(u32 ecode, u8 *desc, u32 length);
STATIC int devdrv_manager_get_errstr(unsigned long arg)
{
    struct bb_err_string user_arg = { 0, 0, 0, { 0 } };
    int ret;

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct bb_err_string));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return -1;
    }

    if ((user_arg.dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (user_arg.buf_len != BBOX_ERRSTR_LEN)) {
        devdrv_drv_err(
            "invalid device id, id = %d. or invalid buffer len, user_arg.buf_len = %d, user_arg.buf_len should be %d\n",
            user_arg.dev_id, user_arg.buf_len, BBOX_ERRSTR_LEN);
        return -1;
    }
    ret = bbox_get_device_ecode_info(user_arg.errcode, user_arg.errstr, user_arg.buf_len);
    if (ret) {
        devdrv_drv_err("get error str failed or the errcode not exist. ret = %d.\n", ret);
        return -1;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &user_arg, sizeof(struct bb_err_string));
    if (ret != 0) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -1;
    }
    return 0;
}

/*
 * register operation, read or write
 */
int devdrv_reg_op(unsigned char op_type, unsigned long base_phy_reg, unsigned long reg_offset,
    unsigned long map_size, unsigned int *val)
{
    int ret;
    void __iomem *regs = NULL;

    regs = ioremap(base_phy_reg, map_size);
    if (regs == NULL) {
        devdrv_drv_err("ioremap error.\n");
        ret = -ENOMEM;
        return ret;
    }

    if (op_type == DEVDRV_REG_WR) {
        writel(*val, (void __iomem *)(uintptr_t)((long)(uintptr_t)regs + reg_offset));
    } else {
        *val = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)regs + reg_offset));
    }

    (void)iounmap(regs);
    regs = NULL;

    return 0;
}

STATIC int devdrv_reset_i2c_controller(unsigned long arg)
{
#ifdef CFG_SOC_PLATFORM_CLOUD
    return -1;
#else
    int ret;
    struct ioctl_arg user_arg = { 0 };
    unsigned int reg_val;
    unsigned int dev_id;

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return -1;
    }

    dev_id = user_arg.dev_id;

    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("invalid device id, id = %d.\n", dev_id);
        return -1;
    }

    /* reset the i2c controller by set the register */
    reg_val = 1;
    ret = devdrv_reg_op(DEVDRV_REG_WR, SM_BUS_CTL_BASE_ADDR, RESET_I2C_OFFSET, I2C_RESET_MAP_SIZE, &reg_val);
    if (ret < 0) {
        devdrv_drv_err("Register operation failed. (ret=%d)\n", ret);
        return -1;
    }

    /* unreset the i2c controller by set the register */
    reg_val = 1;
    ret = devdrv_reg_op(DEVDRV_REG_WR, SM_BUS_CTL_BASE_ADDR, UNRESET_I2C_OFFSET, I2C_RESET_MAP_SIZE, &reg_val);
    if (ret < 0) {
        devdrv_drv_err("Register operation failed. (ret=%d)\n", ret);
        return -1;
    }

    return 0;
#endif
}

STATIC int devdrv_get_xloader_boot_info(unsigned long arg)
{
    int ret;
    unsigned int op_flag;
    unsigned int op_area = 0;
    unsigned int reg_val = 0;
    unsigned int idx = 0;
    struct ioctl_arg user_arg = { 0 };

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return -1;
    }

    op_flag = user_arg.data1;

    if (op_flag == DEV_GET_CURR_BOOT_AREA) {
        /*
          sysctrl reg, 0x1100c3448 byte1:
          0x1--xloader0 boot
          0x2--xloader1 boot
        */
        reg_val = 0;
        ret = devdrv_reg_op(DEVDRV_REG_RD, SYSCTRL_REG_BASE, SC_BAK_DATA14, SIZE_OF_64K, &reg_val);
        if (ret < 0) {
            devdrv_drv_err("Register operation failed. (ret=%d)\n", ret);
            return -1;
        }

        idx = ((reg_val >> 8) & 0xff);
        if ((idx != BOOT_FROM_XLOADER0) && (idx != BOOT_FROM_XLOADER1)) {
            devdrv_drv_err("boot area info err: idx=%u\n", idx);
            user_arg.data3 = ERRNO_NONSUPPORT_ITEM;
            user_arg.data1 = 0;
            goto out;
        }

        op_area = (idx == BOOT_FROM_XLOADER0) ? DEV_UPDATE_XLOADER_AREA0 : DEV_UPDATE_XLOADER_AREA1;
    } else if (op_flag == DEV_CLEAR_BOOT_COUNT) {
        /* upgrade or success, need clear boot count */
        ret = devdrv_reg_op(DEVDRV_REG_RD, SYSCTRL_REG_BASE, SC_SOFT_POR_RSV3, SIZE_OF_64K, &reg_val);
        if (ret < 0) {
            devdrv_drv_err("Register operation failed. (ret=%d)\n", ret);
            return -1;
        }

        reg_val = 0;
        ret = devdrv_reg_op(DEVDRV_REG_WR, SYSCTRL_REG_BASE, SC_SOFT_POR_RSV3, SIZE_OF_64K, &reg_val);
        if (ret < 0) {
            devdrv_drv_err("Register operation failed. (ret=%d)\n", ret);
            return -1;
        }
    } else {
        devdrv_drv_err("dev_upgrade_xloader_boot_area_op fail, op_flag(%x)err.\n", op_flag);
    }

    devdrv_drv_info("op_flag: %x, value: %x, idx: %u\n", op_flag, reg_val, idx);

    user_arg.data1 = op_area;
    user_arg.data3 = 0;

out:
    ret = copy_to_user_safe((void *)(uintptr_t)arg, &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -1;
    }

    return 0;
}

STATIC int devdrv_gpio_read(unsigned long arg)
{
    int ret;
    int gpio_num;
    unsigned int gpio_val;
    struct ioctl_arg user_arg = { 0 };

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return -1;
    }

    gpio_num = user_arg.data1;

    if (!gpio_is_valid(gpio_num)) {
        devdrv_drv_err("invalid gpio, gpio=%d\n", gpio_num);
        return -1;
    }

    ret = gpio_request(gpio_num, DEVDRV_GPIO_NAME);
    if (ret) {
        devdrv_drv_err("gpio [%d] request failed\n", gpio_num);
        return ret;
    }

    gpio_val = gpio_get_value(gpio_num);

    (void)gpio_free(gpio_num);

    user_arg.data1 = gpio_val;
    ret = copy_to_user_safe((void *)(uintptr_t)arg, &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -1;
    }

    return 0;
}


int devdrv_manager_inquiry_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    switch (cmd) {
        case DEVDRV_MANAGER_GET_PCIE_ID_INFO:
            ret = devdrv_get_pcie_id(arg);
            break;
        case DEVDRV_MANAGER_GET_FLASH_COUNT:
            ret = devdrv_get_flash_count(arg);
            break;
        case DEVDRV_MANAGER_GET_FLASH_INFO:
            ret = devdrv_get_flash_info(arg);
            break;
        case DEVDRV_MANAGER_GET_HEALTH_CODE:
            ret = devdrv_get_health_code(arg);
            break;
        case DEVDRV_MANAGER_GET_ERROR_CODE:
            ret = devdrv_get_error_code(arg);
            break;
        case DEVDRV_MANAGER_GET_DDR_CAPACITY:
            ret = devdrv_get_ddr_capacity(arg);
            break;

        default:
            devdrv_drv_err("invalid cmd.\n");
            ret = -1;
            break;
    }
    return ret;
}

int devdrv_manager_inquiry_info_ex(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    switch (cmd) {
        case DEVDRV_MANAGER_RST_I2C_CTROLLER:
            ret = devdrv_reset_i2c_controller(arg);
            break;
        case DEVDRV_MANAGER_GET_XLOADER_BOOT_INFO:
            ret = devdrv_get_xloader_boot_info(arg);
            break;
        case DEVDRV_MANAGER_GET_GPIO_STATE:
            ret = devdrv_gpio_read(arg);
            break;
        default:
            devdrv_drv_err("invalid cmd.\n");
            ret = -1;
            break;
    }

    return ret;
}

int devdrv_manager_imu_cmd(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;

    switch (cmd) {
#ifdef CFG_BUILD_DEBUG
        case DEVDRV_MANAGER_IMU_SMOKE:
            ret = devdrv_imu_smoke_ipc(arg);
            break;
#endif
        case DEVDRV_MANAGER_SEND_TO_IMU:
            ret = devdrv_ipc_send_to_imu(arg);
            break;
        case DEVDRV_MANAGER_RECV_FROM_IMU:
            ret = devdrv_ipc_recv_from_imu(arg);
            break;
#ifdef CFG_BUILD_DEBUG
        case DEVDRV_MANAGER_GET_IMU_INFO:
            ret = devdrv_get_imu_info(arg);
            break;
#endif
        case DEVDRV_MANAGER_CONFIG_ECC_ENABLE:
            ret = devdrv_config_ecc_enable(arg);
            break;
#ifdef CFG_BUILD_DEBUG
        case DEVDRV_MANAGER_DEBUG_INFORM:
            ret = devdrv_imu_debug(arg);
            break;
#endif
        case DEVDRV_MANAGER_COMPUTE_POWER:
            ret = tsmng_get_ai_computing_power(arg);
            break;
        case DEVDRV_MANAGER_GET_BBOX_ERRSTR:
            ret = devdrv_manager_get_errstr(arg);
            break;
        default:
            devdrv_drv_err("invalid cmd.\n");
            ret = -1;
            break;
    }
    return ret;
}

int devdrv_manager_get_pmu_voltage(struct file *filep, unsigned int cmd, unsigned long arg)
{
#ifdef CFG_SOC_PLATFORM_MINI

#ifdef CONFIG_HISI_SPMI_PMIC_VOLTAGE
    struct pmu_voltage_stru pmu_voltage = { 0 };
    unsigned int value = 0;
    int ret;

    if ((void *)(uintptr_t)arg == NULL) {
        devdrv_drv_err("parameter check failed.\n");
        return -EINVAL;
    }

    ret = copy_from_user_safe(&pmu_voltage, (void *)(uintptr_t)arg, sizeof(struct pmu_voltage_stru));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }
    // stub for pmu get interface
    switch (pmu_voltage.pmu_type) {
        case PMU21_VBUCK_VOLTAGE:
            ret = get_main_pmu_buck_volt(pmu_voltage.channel, &value);
            break;
        case PMU21_VOUT_VOLTAGE:
            ret = get_main_pmu_ldo_volt(pmu_voltage.channel, &value);
            break;
        case PMU22_VBUCK_VOLTAGE:
            ret = get_second_pmu_buck_volt(pmu_voltage.device_id, pmu_voltage.channel, &value);
            break;
        case PMU_ADC_VOLTAGE:
            ret = hisi_adc_get_value(pmu_voltage.channel);
            break;
        default:
            devdrv_drv_err("undefine PMU type!:%u\n", pmu_voltage.pmu_type);
            ret = -EINVAL;
            break;
    }

    pmu_voltage.get_value = value;
    pmu_voltage.return_value = ret;

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &pmu_voltage, sizeof(struct pmu_voltage_stru));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -EINVAL;
    }

    return ret;
#else
    return 0;
#endif

#else
    return 0;
#endif
}

/* stub for svm
 * result = 0: no task exist in the TS
 * others: task exist in the TS
 */
int devdrv_inquire_aicore_task(unsigned int dev_id, unsigned int fid, unsigned int tgid,
    unsigned int *result)
{
    rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH] = {0};
    rproc_msg_t ack_buffer[IPCDRV_RPROC_MSG_LENGTH];
    struct ipcdrv_message *ipc_msg = NULL;
    int ret;

    if ((dev_id >= MAX_CHIP_NUM) || (result == NULL)) {
        devdrv_drv_err("invalid parameter, dev_id = %u, result = %pK.\n", dev_id, result);
        return -EINVAL;
    }
    ipc_msg = (struct ipcdrv_message *)msg;
    ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
    ipc_msg->ipc_msg_header.cmd_type = IPCDRV_TS_INQUIRY_AICORE_TASK;
    ipc_msg->ipc_msg_header.sync_type = IPCDRV_MSG_SYNC;
    ipc_msg->ipc_msg_header.reserved = 0;
    ipc_msg->ipc_msg_header.msg_index = 0;
    *(unsigned int *)ipc_msg->ipcdrv_payload = tgid;
    *((unsigned int *)ipc_msg->ipcdrv_payload + 1) = fid;
    ipc_msg->ipc_msg_header.msg_length = sizeof(unsigned int) + sizeof(unsigned int);
    devdrv_drv_debug("inquire aicore_task, fid = %d, pid = %d.\n", fid, tgid);

    ret = icm_msg_send_sync(ICM_FD_BUILD(dev_id, HISI_RPROC_TX_TS), (rproc_msg_t *)&msg, IPCDRV_RPROC_MSG_LENGTH,
        (rproc_msg_t *)ack_buffer, IPCDRV_RPROC_MSG_LENGTH);
    if (ret) {
        devdrv_drv_warn("devdrv_info_ipc_to_ts_sync failed. ret = %d.\n", ret);
        return ret;
    }
    ipc_msg = (struct ipcdrv_message *)ack_buffer;
    *result = *(unsigned int *)ipc_msg->ipcdrv_payload;

    return 0;
}
EXPORT_SYMBOL(devdrv_inquire_aicore_task);

