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

#include <linux/securec.h>
#include <linux/securectype.h>
#include <linux/pinctrl/consumer.h>
#include <linux/errno.h>
#include "devdrv_common.h"
#include "devdrv_user_common.h"
#include "devdrv_manager_common.h"
#include "devdrv_parse_pdata.h"
#include "devmng_dts.h"
#include "drv_notify.h"
#include "drv_comm.h"
#include "ascend_platform.h"

/* Chip info */
#define SOC_CHIP_INFO_REG_BASE 0x8000F000UL
#define SOC_CHIP_INFO_REG_OFFSET 0xFF8
#define SOC_CHIP_INFO_MAP_SIZE 0x1000

STATIC int get_board_id_by_reg(int *board_id)
{
    void __iomem *reg_addr = NULL;
    int reg_value;

    reg_addr = ioremap(SYSCTL_REG_BASE_ADDR + DRV_BOARD_ID_REG_OFFSET, sizeof(int));
    if (reg_addr == NULL) {
        devdrv_drv_err("devm_ioremap failed.\n");
        return -EINVAL;
    }
    reg_value = readl(reg_addr);
    iounmap(reg_addr);
    *board_id = reg_value & BOARD_ID_REG_VALUE_MASK;
    devdrv_drv_info("get board_id success, borad_id = %d.\n", *board_id);
    return 0;
}

STATIC int devdrv_gpio_iomux_cfg(struct device *dev, const char *name)
{
    int ret;
    struct pinctrl *pinctrl = NULL;
    struct pinctrl_state *pins_default = NULL;

    if (dev == NULL) {
        devdrv_drv_err("parameter dev is null.\n");
        return -EINVAL;
    }

    if (name == NULL) {
        devdrv_drv_err("parameter name is null.\n");
        return -EINVAL;
    }

    /*
     * Normally devm_pinctrl_put function will not need to be called,
     * repeatedly calling devm_pinctrl_get will not apply for resources repeatedly,
     * the resource management code will ensure that the resource is freed.
     */
    pinctrl = devm_pinctrl_get(dev);
    if (IS_ERR(pinctrl)) {
        devdrv_drv_err("devm_pinctrl_get failed.\n");
        pinctrl = NULL;
        return -ENOENT;
    }
    pins_default = pinctrl_lookup_state(pinctrl, name);
    if (IS_ERR(pins_default)) {
        devdrv_drv_err("pinctrl_lookup_state %s failed.\n", name);
        devm_pinctrl_put(pinctrl);
        pins_default = NULL;
        return -ENOENT;
    }
    ret = pinctrl_select_state(pinctrl, pins_default);
    if (ret < 0) {
        devdrv_drv_err("pinctrl_select_state %s failed.\n", name);
        devm_pinctrl_put(pinctrl);
        return ret;
    }

    return ret;
}

void devdrv_gpio_iomux_init(struct device *dev)
{
    int ret;
    int board_id;

    if (dev == NULL) {
        devdrv_drv_err("parameter dev is null.\n");
        return;
    }

    ret = get_board_id_by_reg(&board_id);
    if (ret != 0) {
        devdrv_drv_err("get board id form reg fail, (ret = %d).\n", ret);
        return;
    }
    switch (board_id) {
        case BOARD_ID_002:
        case BOARD_ID_059:
            ret = devdrv_gpio_iomux_cfg(dev, PINCTRL_STATE_DEFAULT);
            if (ret != 0) {
                devdrv_drv_err("devdrv_gpio_iomux_cfg failed, board_id: %d.\n", board_id);
                return;
            }
            break;
        default:
            devdrv_drv_info("devdrv gpio iomux ignore, board_id: %d.\n", board_id);
            return;
    }

    return;
}

STATIC int devdrv_check_is_evb_board(u32 board_id)
{
    return (((board_id >= BOARD_ID_900) && (board_id <= BOARD_ID_999)) ? 1 : 0);
}

STATIC int devdrv_check_palt_type_is_asic(void)
{
    void __iomem *reg_addr = NULL;
    int reg_value;

    reg_addr = ioremap(SYSCTL_REG_BASE_ADDR + DEVDRV_PLAT_TYPE_REG_OFFSET, sizeof(int));
    if (reg_addr == NULL) {
        devdrv_drv_err("devm_ioremap failed.\n");
        return -EINVAL;
    }
    reg_value = readl(reg_addr);
    iounmap(reg_addr);
    if (reg_value == 0) {
        devdrv_drv_info("palt_type is asic.\n");
        return 1;
    } else {
        devdrv_drv_info("palt_type is not asic, reg_value(%#x).\n", reg_value);
        return 0;
    }
}

STATIC int devdrv_get_slot_id(int gpio_num, u32 *slot_id)
{
    int ret;
    int board_id;

    ret = get_board_id_by_reg(&board_id);
    if (ret != 0) {
        devdrv_drv_err("get board_id from reg failed, ret = %d.\n", ret);
        return -EINVAL;
    }
    if (devdrv_check_is_evb_board(board_id)) {
        *slot_id = 0;
        devdrv_drv_info("this is evb_board.\n");
        return 0;
    }
    if (!devdrv_check_palt_type_is_asic()) {    // plat_type is ESL/EMU/FPGA
        *slot_id = 0;
        return 0;
    }

    if (!gpio_is_valid(gpio_num)) {
        devdrv_drv_err("invalid gpio, gpio=%d\n", gpio_num);
        return -EINVAL;
    }

    ret = gpio_request(gpio_num, DEVDRV_GPIO_NAME);
    if (ret) {
        devdrv_drv_err("gpio [%d] request failed\n", gpio_num);
        return ret;
    }

    *slot_id = gpio_get_value(gpio_num);

    (void)gpio_free(gpio_num);

    return 0;
}

STATIC int devdrv_get_chip_type_by_reg(int *chip_type)
{
    void __iomem *reg_addr = NULL;
    int reg_value;

    reg_addr = ioremap(SOC_CHIP_INFO_REG_BASE, SOC_CHIP_INFO_MAP_SIZE);
    if (reg_addr == NULL) {
        devdrv_drv_err("devm_ioremap failed.\n");
        return -EINVAL;
    }

    reg_value = readl(reg_addr + SOC_CHIP_INFO_REG_OFFSET);
    iounmap(reg_addr);
    *chip_type = reg_value & CHIP_TYPE_REG_VALUE_MASK;
    devdrv_drv_info("get chip_type success, chip_type = %d.\n", *chip_type);

    return 0;
}


static void devdrv_get_soc_workmode(int *soc_workmode_list)
{
    int ret;
    int i ;
    int slotid = -1;
    int board_id = -1;
    int chip_type = -1;
    struct device_node *np = NULL;
    char *soc_workmode_name[] = {
        "is_actstd",
        "is_loadbalanc",
        "total_chipnum",
        "phy_log_map",
        "domain_mode",
        NULL
    };

    ret = devdrv_get_slot_id(DEVDRV_MDC_GPIO_SLOT_ID, &slotid);
    if (ret != 0) {
        devdrv_drv_warn("can't find valid slotid. ret: %d.\n", ret);
    }

    soc_workmode_list[KEY_CUR_PHYPOS] = slotid;

    ret = devdrv_get_chip_type_by_reg(&chip_type);
    if (ret != 0) {
        devdrv_drv_warn("devdrv_get_chip_type_by_reg failed, ret(%d).\n", ret);
    }
    soc_workmode_list[KEY_CHIP_TYPE] = chip_type;

    np = of_find_compatible_node(NULL, NULL, "socworkmode");
    if (np == NULL) {
        devdrv_drv_warn("can't find valid socworkmode node.\n");
    }

    for (i = 0; soc_workmode_name[i] != NULL; i++) {
        ret = of_property_read_u32(np, soc_workmode_name[i], &soc_workmode_list[i]);
        if (ret != 0) {
            devdrv_drv_warn("can't find valid soc workmode[%d]: %s, ret: %d.\n",
                            i, soc_workmode_name[i], ret);
        }
    }

    switch (soc_workmode_list[KEY_IS_ACTSTD]) {
        case VAL_IS_ACTSTD_MODE_YES:
            soc_workmode_list[KEY_CUR_ACTSTD] = VAL_ACTSTD_STATE_STD;
            break;
        case VAL_IS_ACTSTD_MODE_NO:
            soc_workmode_list[KEY_CUR_ACTSTD] = VAL_ACTSTD_STATE_ACT;
            break;
        default:
            devdrv_drv_warn("can't find valid KEY_CUR_ACTSTD.\n");
            break;
    }

    ret = get_board_id_by_reg(&board_id);
    if (ret != 0) {
        devdrv_drv_warn("get_board_id_by_reg failed, ret = %d.\n", ret);
    }
    soc_workmode_list[KEY_BOARD_ID] = board_id;
    return;
}

static void devdrv_set_soc_workmode(int *soc_workmode_list)
{
    int ret = 0;
    int i;

    for (i = 0; i < KEY_MAX; i++) {
        if (soc_workmode_list[i] != -1) {
            ret = centre_notify_set_val(i, soc_workmode_list[i]);
            if (ret != 0) {
                devdrv_drv_err("centre_notify_set_val failed, i: %d, ret: %d.\n", i, ret);
            }
        }
    }
}

void devdrv_soc_workmode_init()
{
    int soc_workmode_list[KEY_MAX] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};

    devdrv_get_soc_workmode(soc_workmode_list);
    devdrv_set_soc_workmode(soc_workmode_list);

    return;
}
