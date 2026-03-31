/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-08-13
 */

#ifndef BBOX_PLATFORM_H
#define BBOX_PLATFORM_H

#include <linux/io.h>

#include "device/bbox_types.h"

/****************TEST platform*********************************/
#ifdef DEBUG
#define STATIC
#define VOLATILE
#define INIT
#define EXIT
typedef void dev_class_t;
#else
#define STATIC static
#define VOLATILE volatile
#define INIT __init
#define EXIT __exit
typedef struct class dev_class_t;
#endif

/****************BBOX*********************************/

#ifdef BBOX_SOC_PLATFORM_MILAN
#define BBOX_DEVICE_DDR_PARTITION   0x10000000000UL
#define BBOX_DIE_HIGH_OFFSET   0x1000000000UL
#define BBOX_CHIP_DDR_PARTITION    0x80000000000ULL
#elif defined BBOX_SOC_PLATFORM_MDC
#define BBOX_DEVICE_DDR_PARTITION   0x8000000000UL
#define BBOX_DIE_HIGH_OFFSET   0x0UL
#define BBOX_CHIP_DDR_PARTITION    0x0UL
#else
#define BBOX_DEVICE_DDR_PARTITION   0x200000000000UL
#define BBOX_DIE_HIGH_OFFSET   0x0UL
#define BBOX_CHIP_DDR_PARTITION    0x0UL
#endif

// macro abort device
#define DEFAULT_DEVICE_ID   0UL
#define DEVICE_MAX_NUM      4U

static u32 bbox_get_chip_id(void)
{
    u32 chip_id = 0;
#ifdef BBOX_SOC_PLATFORM_MILAN
    void *chip_id_addr = NULL;

    chip_id_addr = ioremap_cache(0x36EFD800, sizeof(u32));  /* 0x36EFD800 chip id addr */
    if (chip_id_addr == NULL) {
        return 0;
    }
    chip_id = readl(chip_id_addr) & 0xFF;

    iounmap(chip_id_addr);
    chip_id_addr = NULL;

    if (chip_id >= 8) { /* chip id < 8 */
        return 0;
    }
#endif
    return chip_id;
}

static bool bbox_is_pcie_connet(void)
{
    bool is_pcie_connet = true;
#ifdef BBOX_SOC_PLATFORM_MILAN
    /* bios set, drv use */
    struct devdrv_hw_info {
        unsigned char chip_id;
        unsigned char multi_chip;
        unsigned char multi_die;
        unsigned char mainboard_id;
        unsigned short hccs_connect_status;
        unsigned short board_id;
        unsigned int reserved;
    };
    struct devdrv_hw_info *hw_data = NULL;
    /* 0x36EFD800 chip id addr */
    hw_data = (struct devdrv_hw_info *)ioremap_cache(0x36EFD800, sizeof(struct devdrv_hw_info));
    if (hw_data == NULL) {
        return true;
    }
    is_pcie_connet = (hw_data->hccs_connect_status == 0) ? true : false;
    iounmap(hw_data);
#endif
    return is_pcie_connet;
}

static inline u64 bbox_get_pcie_connet_device_addr(u32 devid, u64 offset)
{
    if (devid >= DEVICE_MAX_NUM) {
        return 0;
    } else if (devid > 0) {
        return (bbox_get_chip_id() * BBOX_CHIP_DDR_PARTITION + devid * BBOX_DEVICE_DDR_PARTITION
            + BBOX_DIE_HIGH_OFFSET + offset);
    } else {
        return ((devid * BBOX_DEVICE_DDR_PARTITION) + offset);
    }
}

static inline u64 bbox_get_hccs_connet_device_addr(u32 devid, u64 offset)
{
#define BBOX_HCCS_DIE0_ADD_OFFSET_LIMIT     0x80000000ULL    /* For die 0, less this limit should add  */
#define BBOX_HCCS_CHIP_DDR_PARTITION        0x20000000000ULL
#define BBOX_HCCS_BASE_OFFSET               0x200000000000ULL
    u64 hccs_base_offset, hccs_die_offset, hccs_chip_offset;

    hccs_base_offset = ((devid == 0) && (offset < BBOX_HCCS_DIE0_ADD_OFFSET_LIMIT)) ?
        0 : BBOX_HCCS_BASE_OFFSET;
    hccs_die_offset = devid * (BBOX_DEVICE_DDR_PARTITION + BBOX_DIE_HIGH_OFFSET);
    hccs_chip_offset = ((devid == 0) && (offset < BBOX_HCCS_DIE0_ADD_OFFSET_LIMIT)) ?
        0 : (u64)bbox_get_chip_id() * BBOX_HCCS_CHIP_DDR_PARTITION;

    return offset + hccs_base_offset + hccs_die_offset + hccs_chip_offset;
}

static inline u64 bbox_get_device_addr(u32 devid, u64 offset)
{
    if (bbox_is_pcie_connet()) {
        return bbox_get_pcie_connet_device_addr(devid, offset);
    } else {
        return bbox_get_hccs_connet_device_addr(devid, offset);
    }
}

/* multiple die use the same os, reset register only use die 0 */
static inline u64 bbox_register_get_device_addr(u32 devid, u64 offset)
{
    if (devid >= DEVICE_MAX_NUM) {
        return 0;
    }
    if (bbox_is_pcie_connet()) {
        return (bbox_get_chip_id() * BBOX_CHIP_DDR_PARTITION + devid * BBOX_DEVICE_DDR_PARTITION + offset);
    } else {
        return bbox_get_device_addr(devid, offset);
    }
}

#endif // BBOX_PLATFORM_H
