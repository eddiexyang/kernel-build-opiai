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

#ifndef SDK_HILINK_CFG_H
#define SDK_HILINK_CFG_H

#include <linux/io.h>
#include <linux/types.h>
#include "sdk_hilink_pub.h"

#define DRV_SRAM_BASE 0x90300000
#define DRV_SRAM_ELASTIC_CFG_OFFSET 0x17400
#define DRV_SRAM_ELASTIC_CFG_SIZE 0x4C00
#define DRV_SRAM_ELASTIC_CFG_BIOS_MAGIC 0x53484D47
#define DRV_SRAM_ELASTIC_CFG_END_NODE_ID 0xFFFFABCD

#ifdef ELASTIC_CFG_OFFSET_ENABLE
#define DRV_DDR_ELASTIC_CFG_SYS_OFFSET 0x1F80000
#define DRV_DDR_ELASTIC_CFG_USER_OFFSET 0x1F90000
#else
#define DRV_DDR_ELASTIC_CFG_SYS_OFFSET 0x3600000
#define DRV_DDR_ELASTIC_CFG_USER_OFFSET 0x3610000
#endif

#define DRV_DDR_ELASTIC_CFG_SIZE 0x10000
#define DRV_DDR_ELASTIC_CFG_MODULE_MAGIC 0x606F6475

typedef enum {
    PHY_PG_INFO_ID = 0,
    BIN_DOWN_INFO_ID,
    SERDES_INFO_ID,
    LP_INFO_ID,
    RECOVERY_INFO_ID,
    HSM_INFO_ID,
    DDR_INFO_ID,
} CFG_INFO_NODE_ID;

typedef enum {
    DRV_CFG_SYS_MOUDLE_ID_PG = 0,
    DRV_CFG_SYS_MOUDLE_ID_SERDES,
    DRV_CFG_SYS_MOUDLE_ID_LP,
    DRV_CFG_SYS_MOUDLE_ID_RECOVERY,
    DRV_CFG_SYS_MOUDLE_ID_DDR,
    DRV_CFG_SYS_MOUDLE_ID_INVALID
} CFG_SYS_MODULE_ID;

typedef enum {
    DRV_CFG_USER_MOUDLE_ID_SERDES = 0,
    DRV_CFG_USER_MOUDLE_ID_RECOVERY,
    DRV_CFG_USER_MOUDLE_ID_INVALID
} CFG_USER_MODULE_ID;

typedef enum {
    SERDES_TYPE_PCIE = 0,
    SERDES_TYPE_SATA,
    SERDES_TYPE_USB,
    SERDES_TYPE_ETH,
    SERDES_TYPE_NULL
} DRV_SERDES_TYPE;

typedef enum {
    SERDES_FREQ_ETH_1_25G = 0,
    SERDES_FREQ_ETH_3_125G,
    SERDES_FREQ_PCIE_GEN_1_2,
    SERDES_FREQ_PCIE_GEN_3,
    SERDES_FREQ_SATA_6G,
    SERDES_FREQ_USB_5G,
    SERDES_FREQ_NULL
} DRV_SERDES_FREQUENCY_LEVEL;

/* SHMG in SRAM */
#define SHMG_RESV_SIZE 5
struct drv_cfg_shmg_head {
    u32 magic;          // ascii:SHMG 0x53484D47
    u32 version;        // 100 means v1.00
    u32 head_size;      // this struct size
    u32 share_mem_size; // this node share mem size
    u32 reserved[SHMG_RESV_SIZE];
    char node_list[0];
};

struct drv_cfg_uni_node_head {
    u32 magic;      // asic:BSBC BIOS DDR_ LP__ HSM_ SILS
    u32 node_id;    // DRV_ELASTIC_CFG_INFO_NODE_ID
    u32 version;    // 100 means v1.00
    u32 node_size;  // XXX_NODE_INFO size
    u32 check_sum;  // XXX_RAW_DATA sum by 4byte
    char node_data[0];
};

struct drv_cfg_uni_node_raw_data {
    u32 sys_addr;
    u32 user_addr;
};

struct drv_cfg_serdes_node_info {
    struct drv_cfg_uni_node_head head;
    struct drv_cfg_uni_node_raw_data raw_data;
};

/* cfg in DDR */
struct drv_cfg_fix_mod_head {
    u32 magic;         // fix value:"modu"aisc code,0x606F6475
    u32 mod_len;       // module tool fill, frame use, modHdr + modData
    u32 mod_id;        // module tool fill，frame use，module may use
    u32 mod_ver;       // module tool fill, frame not use
    u32 mod_head_size; // module tool fill，module use,
    u32 check_sum;     // module tool fill, frame not use,modHdr(except checksum) + modData
};

struct drv_cfg_serdes_lane_info {
    /* enable: 1; disable: 0 */
    u32 flag;
    u32 lan_index;
    /* multiplexing type e.g. usb，pcie */
    DRV_SERDES_TYPE type;
    /* enable 1：disable：0 */
    u32 ssc_enable;
    /* polarity:0   reversal:1 */
    u32 polarity_tx;
    /* polarity:0   reversal:1 */
    u32 polarity_rx;
    /* lan order, positive sequence: 0, inverted sequence:1 */
    u32 lan_order;
    /* bandwidth size e.g. If the type is PCIE, the value is 2 or 4, meaning is x2 or x4, Invalid value is 0xff */
    u32 bandwidth;
    /* 0 : master,  1: salver,  Invalid value is 0xff. currently not using */
    u32 align_mode;
    DRV_SERDES_FREQUENCY_LEVEL frequency;
    /* mapping between application ports and lanes */
    u32 port_index;
};

struct drv_cfg_serdes_raw_data {
    struct drv_cfg_fix_mod_head mod_head;
    struct drv_cfg_serdes_lane_info data[MACRO_MAX * LANE_MAX];
};

struct hilink_cs_cfg {
    u32 ssc_enable;
    DRV_SERDES_TYPE type;
    DRV_SERDES_FREQUENCY_LEVEL frequency;
};

struct hilink_ds_cfg {
    u32 port;
    u32 cs_select;
    u32 ssc_enable;
    u32 polarity_tx;
    u32 polarity_rx;
    DRV_SERDES_TYPE type;
};

struct hilink_macro_cfg {
    u32 lane_order;
    struct hilink_cs_cfg cs[CS_MAX];
    struct hilink_ds_cfg ds[DS_MAX];
};

int hilink_elastic_cfg_init(void);
bool hilink_cfg_is_exist_prot(struct hilink_macro_cfg *macro_cfg, DRV_SERDES_TYPE type);
int hilink_get_macro_cfg_by_board_id(struct hilink_macro_cfg *macro_cfg, u32 macro, u32 board_id);
#endif