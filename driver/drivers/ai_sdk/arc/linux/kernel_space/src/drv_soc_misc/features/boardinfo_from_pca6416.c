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

#include <linux/kallsyms.h>
#include <linux/version.h>
#include <linux/module.h>
#include "PCA6416.h"
#include "devdrv_interface.h"
#include "soc_misc_board_info.h"
#include "soc_misc_init.h"

STATIC int soc_check_support_pca6416(struct soc_misc_info_st *soc_info)
{
    unsigned int board_id;

    board_id = soc_info->board_info.board_id;
    if ((board_id == DEV_BOARDID_PCIE_VA) || (board_id == DEV_BOARDID_PCIE_VC) || (board_id == DEV_BOARDID_PCIE_VD) ||
        ((board_id >= DEV_BOARDID_PCIE_LOW_LIMIT) && (board_id <= DEV_BOARDID_PCIE_UPPER_LIMIT)) ||
        ((board_id >= DEV_BOARDID_EVB_LOW_LIMIT) && (board_id <= DEV_BOARDID_EVB_UPPER_LIMIT))) {
        return -EINVAL;
    }

    return 0;
}

enum board_id_type {
    PCB_ID,
    BOM_ID,
};
typedef int (*get_board_info_func)(int id_type, unsigned int *val);
#define PCA6416_GET_BOARD_INFO "PCA6416_get_board_info"

int soc_misc_init_pcb_id_from_pca6416(struct soc_misc_info_st *soc_info)
{
    get_board_info_func pca6416_get_board_info_func = NULL;
    unsigned int pcb_id = 0;
    int ret;

    if (soc_info == NULL) {
        soc_misc_drv_err("soc info is NULL.\n");
        return -EINVAL;
    }

    ret = soc_check_support_pca6416(soc_info);
    if (ret != 0) {
        goto OUT;
    }
    pca6416_get_board_info_func = (get_board_info_func)(uintptr_t)__symbol_get(PCA6416_GET_BOARD_INFO);
    if (pca6416_get_board_info_func == NULL) {
        goto OUT;
    }

    ret = pca6416_get_board_info_func(PCB_ID, &pcb_id);
    __symbol_put(PCA6416_GET_BOARD_INFO);
    if (ret != 0) {
        soc_misc_drv_err("Get pcb id from pca6416 failed. (dev_id=%u; ret=%d)\n", soc_info->dev_id, ret);
    }

OUT:
    soc_info->board_info.pcb_id = pcb_id;
    return 0;
}
int soc_misc_init_bom_id_from_pca6416(struct soc_misc_info_st *soc_info)
{
    get_board_info_func pca6416_get_board_info_func = NULL;
    unsigned int bom_id = 0;
    int ret;

    if (soc_info == NULL) {
        soc_misc_drv_err("soc info is NULL.\n");
        return -EINVAL;
    }

    ret = soc_check_support_pca6416(soc_info);
    if (ret != 0) {
        goto OUT;
    }

    pca6416_get_board_info_func = (get_board_info_func)(uintptr_t)__symbol_get(PCA6416_GET_BOARD_INFO);
    if (pca6416_get_board_info_func == NULL) {
        goto OUT;
    }

    ret = pca6416_get_board_info_func(BOM_ID, &bom_id);
    __symbol_put(PCA6416_GET_BOARD_INFO);
    if (ret != 0) {
        soc_misc_drv_err("Get pcb id from pca6416 failed. (dev_id=%u; ret=%d)\n", soc_info->dev_id, ret);
    }

OUT:
    soc_info->board_info.bom_id = bom_id;
    return 0;
}
