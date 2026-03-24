/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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

#include <asm/io.h>
#include <linux/of_address.h>
#include <linux/of.h>

#include "nvme_comm_drv.h"
#include "devdrv_util.h"
#include "devdrv_interface.h"

#define NVME_INT_REQ_OFFSET 0
#define NVME_INT_VECTOR_OFFSET 2

/*
reg 0:
bit0: WR PF SQ doorbell0 int
bit1: WR PF SQ doorbell1 int
…
bit31: WR PF SQ doorbell31 int

reg 1:
bit0: WR PF SQ doorbell32 int
bit1: WR PF SQ doorbell33 int
…
bit31: WR PF SQ doorbell63 int


reg 3:
bit0: WR PF SQ doorbell96 int
bit1: WR PF SQ doorbell97 int
…
bit31: WR PF SQ doorbell127 int
*/
#define AGENTDRV_SINGLE_REG_CTRL_DB_NUM 32
#define AGENTDRV_SINGLE_REG_CTRL_IRQ_DB_NUM 4
#define AGENTDRV_DB_REG_NUM 4
#define DB_REG_OFFSET(db_id) ((db_id) / AGENTDRV_SINGLE_REG_CTRL_DB_NUM)
#define DB_BIT_OFFSET(db_id) ((db_id) % AGENTDRV_SINGLE_REG_CTRL_DB_NUM)

/*
0,8,16,…,120           11
1,9,17,…,121           12
2,10,18,…,122          13
3,11,19,…,123          14
4,12,20,…,124          15
5,13,21,…,125          16
6,14,22,…,126          17
7,15,23,…,127          18
*/
u32 g_irq0_db[AGENTDRV_QCNT_EACH_IRQ] = {
    0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120
};
u32 g_irq_db_sts[AGENTDRV_NVME_DB_IRQ_NUM] = {
    0x01010101, 0x02020202, 0x04040404, 0x08080808, 0x10101010, 0x20202020, 0x40404040, 0x80808080
};

#define REG_SIZE 4
u32 nvme_pf_num[DEVDRV_MAX_DEVICE];

spinlock_t g_nvme_doorbell_lock[AGENTDRV_DB_REG_NUM];

void agentdrv_nvme_doorbell_lock_init(void)
{
    int i;

    for (i = 0; i < AGENTDRV_DB_REG_NUM; i++) {
        spin_lock_init(&g_nvme_doorbell_lock[i]);
    }
}

void devdrv_set_nvme_pf_num(u32 devid, u32 pf_num)
{
    nvme_pf_num[devid] = pf_num;
}

u32 devdrv_get_nvme_pf_num(u32 devid)
{
    return DEVDRV_PF_NUM;
}

int devdrv_get_chip_type(void)
{
    return HISI_MINI_V1;
}

#define AGENTDRV_BOARDID_NUM 4
#define AGENTDRV_BOARDID_0 0
#define AGENTDRV_BOARDID_1 1
#define AGENTDRV_BOARDID_2 2
#define AGENTDRV_BOARDID_3 3
#define AGENTDRV_BOARDID_0_RATIO 1000
#define AGENTDRV_BOARDID_1_RATIO 100
#define AGENTDRV_BOARDID_2_RATIO 10

int devdrv_get_board_type(void)
{
    struct device_node *np = NULL;
    unsigned int boardid[AGENTDRV_BOARDID_NUM] = {0};
    u32 board_id;
    int board_type;
    int ret;

    np = of_find_compatible_node(NULL, NULL, "hisilicon,mini");
    if (np == NULL) {
        return BOARD_MINI_EVB;
    }

    ret = of_property_read_u32_array(np, "hisi,boardid", boardid, AGENTDRV_BOARDID_NUM);
    if (ret) {
        return BOARD_MINI_EVB;
    }

    board_id = boardid[AGENTDRV_BOARDID_0] * AGENTDRV_BOARDID_0_RATIO +
               boardid[AGENTDRV_BOARDID_1] * AGENTDRV_BOARDID_1_RATIO +
               boardid[AGENTDRV_BOARDID_2] * AGENTDRV_BOARDID_2_RATIO + boardid[AGENTDRV_BOARDID_3];

    switch (board_id) {
        case BOARD_MINI_PCIE_CARD_BOARDID:
            board_type = BOARD_MINI_PCIE_CARD;
            break;
        case BOARD_MINI_MDC_BOARDID:
            board_type = BOARD_MINI_MDC;
            break;
        case BOARD_MINI_EVB_BOARDID_900:
        case BOARD_MINI_EVB_BOARDID_901:
        case BOARD_MINI_EVB_BOARDID_902:
            board_type = BOARD_MINI_EVB;
            break;
        default:
            board_type = BOARD_MINI_OTHERS;
            break;
    }

    return board_type;
}

void devdrv_raise_int_to_h(void __iomem *io_base, u32 pf, u32 vf, u32 irq)
{
    u32 regval;

    regval = ((irq << NVME_INT_VECTOR_OFFSET) | (1 << NVME_INT_REQ_OFFSET));

    devdrv_nvme_reg_wr(io_base, AGENTDRV_NVME_INT_REQ, regval);
}

void devdrv_get_nvme_irq_db(void __iomem *io_base, u32 sts_base, u32 irq_num, u32 db_id[], u32 *db_num)
{
    u32 i, j, num, regval, db_base, offset;

    num = 0;
    for (i = 0; i < AGENTDRV_DB_REG_NUM; i++) {
        offset = sts_base + i * REG_SIZE;
        devdrv_nvme_reg_rd(io_base, offset, &regval);

        regval &= g_irq_db_sts[irq_num];
        if (regval == 0)
            continue;
        devdrv_nvme_reg_wr(io_base, offset, regval);

        db_base = i * AGENTDRV_SINGLE_REG_CTRL_IRQ_DB_NUM;
        regval >>= irq_num;

        for (j = 0; j < AGENTDRV_SINGLE_REG_CTRL_IRQ_DB_NUM; j++) {
            if (regval & 0x1)
                /*lint -e679 */
                db_id[num++] = g_irq0_db[db_base + j] + irq_num;
            /*lint +e679 */
            regval >>= 8;
        }
    }
    *db_num = num;
}

void devdrv_get_nvme_irq_sq_db(void __iomem *io_base, u32 irq_num, u32 db_id[], u32 func_id, u32 *db_num)
{
    (void)func_id;
    devdrv_get_nvme_irq_db(io_base, AGENTDRV_NVME_SQ_DB_STS_BASE, irq_num, db_id, db_num);
}

void devdrv_get_nvme_irq_cq_db(void __iomem *io_base, u32 irq_num, u32 db_id[], u32 func_id, u32 *db_num)
{
    (void)func_id;
    devdrv_get_nvme_irq_db(io_base, AGENTDRV_NVME_CQ_DB_STS_BASE, irq_num, db_id, db_num);
}

void devdrv_set_nvme_irq_mask(void __iomem *io_base, u32 func_id, u32 irq_num)
{
    u32 reg_offset;
    u32 i, j;
    u32 regval;
    u32 value;
    (void)func_id;

    j = 0;
    for (i = 0; i < AGENTDRV_DB_REG_NUM; i++) {
        spin_lock(&g_nvme_doorbell_lock[i]);

        /*lint -e679 */
        value = ((0x1 << ((g_irq0_db[j] + irq_num) % 32)) | (0x1 << ((g_irq0_db[j + 1] + irq_num) % 32)) |
                 (0x1 << ((g_irq0_db[j + 2] + irq_num) % 32)) | (0x1 << ((g_irq0_db[j + 3] + irq_num) % 32)));
        /*lint +e679 */
        /* sq */
        reg_offset = AGENTDRV_NVME_SQ_DB_INT_MASK_BASE + i * REG_SIZE;
        devdrv_nvme_reg_rd(io_base, reg_offset, &regval);
        regval |= value;
        devdrv_nvme_reg_wr(io_base, reg_offset, regval);

        /* cq */
        reg_offset = AGENTDRV_NVME_CQ_DB_INT_MASK_BASE + i * REG_SIZE;
        devdrv_nvme_reg_rd(io_base, reg_offset, &regval);
        regval |= value;
        devdrv_nvme_reg_wr(io_base, reg_offset, regval);

        spin_unlock(&g_nvme_doorbell_lock[i]);
        j += 4;
    }
}

void devdrv_set_nvme_irq_unmask(void __iomem *io_base, u32 func_id, u32 irq_num)
{
    u32 reg_offset;
    u32 i, j;
    u32 regval;
    u32 value;
    (void)func_id;

    j = 0;
    for (i = 0; i < AGENTDRV_DB_REG_NUM; i++) {
        spin_lock(&g_nvme_doorbell_lock[i]);

        /*lint -e679 */
        value = ~((u32)((0x1 << ((g_irq0_db[j] + irq_num) % 32)) | (0x1 << ((g_irq0_db[j + 1] + irq_num) % 32)) |
                        (0x1 << ((g_irq0_db[j + 2] + irq_num) % 32)) | (0x1 << ((g_irq0_db[j + 3] + irq_num) % 32))));
        /*lint +e679 */
        /* sq */
        reg_offset = AGENTDRV_NVME_SQ_DB_INT_MASK_BASE + i * REG_SIZE;
        devdrv_nvme_reg_rd(io_base, reg_offset, &regval);
        regval &= value;
        devdrv_nvme_reg_wr(io_base, reg_offset, regval);

        /* cq */
        reg_offset = AGENTDRV_NVME_CQ_DB_INT_MASK_BASE + i * REG_SIZE;
        devdrv_nvme_reg_rd(io_base, reg_offset, &regval);
        regval &= value;
        devdrv_nvme_reg_wr(io_base, reg_offset, regval);

        spin_unlock(&g_nvme_doorbell_lock[i]);
        j += 4;
    }
}

void devdrv_set_nvme_irq_enbale(void __iomem *io_base, u32 func_id, u32 irq_num)
{
    devdrv_set_nvme_irq_unmask(io_base, func_id, irq_num);
}

int agentdrv_nvme_irq_num2func(u32 irq_num, u32 *func_id)
{
    (void)irq_num;
    *func_id = 0;
    return 0;
}

int devdrv_nvme_agent_irq_num2vector(u32 irq_num, u32 *irq_vector)
{
    if ((irq_num < AGENTDRV_NVME_DB_IRQ_BASE) || (irq_num >= (AGENTDRV_NVME_DB_IRQ_BASE + AGENTDRV_NVME_DB_IRQ_NUM))) {
        return -1;
    } else {
        *irq_vector = irq_num - AGENTDRV_NVME_DB_IRQ_BASE;
        return 0;
    }
}

u32 devdrv_nvme_agent_irq_vector2num(u32 irq_vector)
{
    return irq_vector + AGENTDRV_NVME_DB_IRQ_BASE;
}

u32 devdrv_nvme_get_cpu_id(u32 start_id, u32 total_num, u32 offset)
{
    return start_id + offset % total_num;
}

void devdrv_soc_func2pfvf(u32 devid, u32 func_id, u32 rdie, struct agentdrv_pfvf_info *pfvf_info)
{
    pfvf_info->pf_num = DEVDRV_PF_NUM;
    pfvf_info->func_num = 0;
    pfvf_info->vf_num = 0;
    pfvf_info->vf_en = 0;
}

bool agentdrv_get_soc_doorbell_capability(void)
{
    return false;
}

bool agentdrv_get_sriov_capability(void)
{
    return false;
}

bool devdrv_is_sriov_enabled(u32 func_totl)
{
    (void)func_totl;
    return false;
}
