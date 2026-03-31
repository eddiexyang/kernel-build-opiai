/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Description: qos
 * Author: huawei
 * Create: 2023-09-15
 */

#ifndef __HCLGE_PLF_QOS_H
#define __HCLGE_PLF_QOS_H

#ifndef CONFIG_PLATFORM_MDC

#include "ascend_kernel_hal.h"
#include "securec.h"
#include "hclge_plf_main.h"

#define QOS_NODE_NAME_MAX_LEN 256
#define MAX_PORT_NUM          4

#define OTSD_ENABLE 1

#define DISABLE_OTSD_LIMIT     0
#define READ_WRITE_MERGE       1
#define READ_WRITE_NOT_MERGE   2

#define QOS_ALLOW_LVL_IDX_0 0
#define QOS_ALLOW_LVL_IDX_1 1
#define QOS_ALLOW_LVL_IDX_2 2

#define DISABLE_BP  0
#define RESPONSE_BP 2

#define QOS_ALLOW_CTRL_ALL   0
#define QOS_ALLOW_CTRL_READ  1
#define QOS_ALLOW_CTRL_WRITE 2

/* SCHE reg addr */
#define SCHE_AR_OSTD_PORT0_CTRL_REG_OFFSET  0x0388
#define SCHE_AW_OSTD_PORT0_CTRL_REG_OFFSET  0x03a8
#define SCHE_SRC_OSTD_PORT0_CTRL_REG_OFFSET 0x02d4

/* outstanding */
#define PPE_TPU_TPU_OUTSTAND_CTRL_REG_DEFAULT       0x10
#define PPE_RPU_RPU_WR_AXI_FIFO_THD1_REG_DEFAULT    0x2d02
#define SCHE_SRC_OSTD_PORT0_CTRL_REG_DEFAULT        0x8040

/* qos */
#define PPE_RCB_COM_RCB_AXQOS_CFG_REG_DEFAULT       0x0
#define PPE_TPU_TPU_CFG_BUS_QOS_REG_DEFAULT         0x0
#define PPE_RPU_RPU_AXQOS_CFG_REG_DEFAULT           0x0
#define PPE_TPU_TPU_DDR_SMMU_USER1_0_REG_DEFAULT    0x0
#define TPU_PORT_OFFSET                             0x20

/* response */
#define SCHE_AR_OSTD_PORT0_CTRL_REG_DEFAULT 0x04404040
#define SCHE_AW_OSTD_PORT0_CTRL_REG_DEFAULT 0x04404040

/* store */
enum {
    PPE_TPU_TPU_OUTSTAND_CTRL_REG_IDX,
    PPE_RPU_RPU_WR_AXI_FIFO_THD1_REG_IDX,
    SCHE_SRC_OSTD_PORT0_CTRL_REG_IDX,
    PPE_RCB_COM_RCB_AXQOS_CFG_REG_IDX,
    PPE_TPU_TPU_CFG_BUS_QOS_REG_IDX,
    PPE_RPU_RPU_AXQOS_CFG_REG_IDX,
    PPE_TPU_TPU_DDR_SMMU_USER1_0_REG_IDX,
    SCHE_AR_OSTD_PORT0_CTRL_REG_IDX,
    SCHE_AW_OSTD_PORT0_CTRL_REG_IDX,
    QOS_REG_NUM
};

int hclge_plf_qos_node_register(void);
void hclge_plf_qos_node_unregister(void);
void hclge_plf_update_qos_node(struct hclge_plf_dev *hdev);

#endif
#endif