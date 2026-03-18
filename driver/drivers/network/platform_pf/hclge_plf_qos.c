/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Description: qos
 * Author: huawei
 * Create: 2023-09-15
 */

#ifndef CONFIG_PLATFORM_MDC

#include "reg_tpu_offset.h"
#include "reg_rpu_offset.h"
#include "reg_rcb_com_offset.h"
#include "hclge_plf_qos.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

static struct qos_master_node g_node_nic;
static struct qos_master_node g_node_tpu;
static struct qos_master_node g_node_rpu;

static atomic_t g_stored_reg_val[QOS_REG_NUM] = {ATOMIC_INIT(PPE_TPU_TPU_OUTSTAND_CTRL_REG_DEFAULT),
    ATOMIC_INIT(PPE_RPU_RPU_WR_AXI_FIFO_THD1_REG_DEFAULT),
    ATOMIC_INIT(SCHE_SRC_OSTD_PORT0_CTRL_REG_DEFAULT),
    ATOMIC_INIT(PPE_RCB_COM_RCB_AXQOS_CFG_REG_DEFAULT),
    ATOMIC_INIT(PPE_TPU_TPU_CFG_BUS_QOS_REG_DEFAULT),
    ATOMIC_INIT(PPE_RPU_RPU_AXQOS_CFG_REG_DEFAULT),
    ATOMIC_INIT(PPE_TPU_TPU_DDR_SMMU_USER1_0_REG_DEFAULT),
    ATOMIC_INIT(SCHE_AR_OSTD_PORT0_CTRL_REG_DEFAULT),
    ATOMIC_INIT(SCHE_AW_OSTD_PORT0_CTRL_REG_DEFAULT)};

STATIC void set_g_stored_reg_val(int idx, u32 val)
{
    atomic_set(&g_stored_reg_val[idx], val);
}

STATIC u32 get_g_stored_reg_val(int idx)
{
    return atomic_read(&g_stored_reg_val[idx]);
}

STATIC int hclge_plf_nic_set_qos_cfg(int devid, const struct qos_master_config_type *cfg)
{
    struct hclge_plf_dev *hdev;
    u32 reg_val;

    if (devid != 0) {
        pr_err("[hclge_plf]:Invalid devid %d\n", devid);
        return -EINVAL;
    }

    if (cfg == NULL) {
        pr_err("[hclge_plf]:cfg is null!\n");
        return -EINVAL;
    }

    if (cfg->type != MASTER_NIC) {
        pr_err("[hclge_plf]:Invalid master %d\n", cfg->type);
        return -EINVAL;
    }

    hdev = hclge_get_first_hclge_plf_dev();
    if (hdev == NULL) {
        pr_err("hns3: hclge get hclge plf dev fail.\n");
        return -EIO;
    }

    if (cfg->qos == 0) {
        set_g_stored_reg_val(PPE_RCB_COM_RCB_AXQOS_CFG_REG_IDX, PPE_RCB_COM_RCB_AXQOS_CFG_REG_DEFAULT);
        set_g_stored_reg_val(PPE_TPU_TPU_CFG_BUS_QOS_REG_IDX, PPE_TPU_TPU_CFG_BUS_QOS_REG_DEFAULT);
        set_g_stored_reg_val(PPE_RPU_RPU_AXQOS_CFG_REG_IDX, PPE_RPU_RPU_AXQOS_CFG_REG_DEFAULT);
        set_g_stored_reg_val(PPE_TPU_TPU_DDR_SMMU_USER1_0_REG_IDX, PPE_TPU_TPU_DDR_SMMU_USER1_0_REG_DEFAULT);
    } else {
        if (cfg->qos > 0xF) {
            pr_err("[hclge_plf]: Invalid qos value %d\n", cfg->qos);
            return -EINVAL;
        }

        reg_val = cfg->qos | (cfg->qos << 4U);
        set_g_stored_reg_val(PPE_RCB_COM_RCB_AXQOS_CFG_REG_IDX, reg_val);

        reg_val = cfg->qos | (cfg->qos << 4U) | (cfg->qos << 8U) | (cfg->qos << 12U);
        set_g_stored_reg_val(PPE_TPU_TPU_CFG_BUS_QOS_REG_IDX, reg_val);
        set_g_stored_reg_val(PPE_RPU_RPU_AXQOS_CFG_REG_IDX, reg_val);

        reg_val = cfg->mpamid | (cfg->pmg << 8U);
        set_g_stored_reg_val(PPE_TPU_TPU_DDR_SMMU_USER1_0_REG_IDX, reg_val);
    }

    if (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state) || test_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state) == 0) {
        pr_warn("[hclge_plf]: try to read reg while resetting!\n");
        return -EBUSY;
    }

    set_bit(HCLGE_STATE_QOS_CFG_CHANGED, &hdev->state);
    return 0;
}

STATIC int hclge_plf_nic_get_qos_cfg(int devid, struct qos_master_config_type *cfg)
{
#define CFG_DDR_PMG_BITS_NUM 3

    struct hclge_plf_dev *hdev;
    u32 reg_addr;
    u32 reg_val;

    if (devid != 0) {
        pr_err("[hclge_plf]:Invalid devid %d\n", devid);
        return -EINVAL;
    }

    if (cfg == NULL) {
        pr_err("[hclge_plf]:cfg is null!\n");
        return -EINVAL;
    }

    if (cfg->type != MASTER_NIC) {
        pr_err("[hclge_plf]:Invalid master %d\n", cfg->type);
        return -EINVAL;
    }

    hdev = hclge_get_first_hclge_plf_dev();
    if (hdev == NULL) {
        pr_err("hns3: hclge get hclge plf dev fail.\n");
        return -EIO;
    }

    if (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state) || test_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state) == 0) {
        pr_err("[hclge_plf]: try to read reg while resetting!\n");
        return -EBUSY;
    }

    reg_addr = RCB_COM_REG_BASE + PPE_RCB_COM_RCB_AXQOS_CFG_REG;
    reg_val = hclge_read_dev(&hdev->hw, reg_addr);
    cfg->qos = reg_val & 0xF;

    reg_addr = TPU_REG_BASE + PPE_TPU_TPU_DDR_SMMU_USER1_0_REG;
    reg_val = hclge_read_dev(&hdev->hw, reg_addr);
    cfg->mpamid = reg_val & 0xFF;
    cfg->pmg = (reg_val >> 8U) & CFG_DDR_PMG_BITS_NUM;

    return 0;
}

/* static */
STATIC int hclge_plf_nic_set_otsd_cfg(int devid, const struct qos_otsd_config_type *cfg)
{
    struct hclge_plf_dev *hdev;
    u32 reg_val;

    if (devid != 0) {
        pr_err("[hclge_plf]:Invalid devid %d\n", devid);
        return -EINVAL;
    }

    if (cfg == NULL) {
        pr_err("[hclge_plf]:cfg is null!\n");
        return -EINVAL;
    }

    if (cfg->master != MASTER_NIC) {
        pr_err("[hclge_plf]:Invalid master %d\n", cfg->master);
        return -EINVAL;
    }

    hdev = hclge_get_first_hclge_plf_dev();
    if (hdev == NULL) {
        pr_err("hns3: hclge get hclge plf dev fail.\n");
        return -EIO;
    }

    switch (cfg->otsd_mode) {
        /* 0 -- disable otsd limit, 1 -- read & write merge, 2 -- read & write not merge */
        case DISABLE_OTSD_LIMIT:
            set_g_stored_reg_val(SCHE_SRC_OSTD_PORT0_CTRL_REG_IDX, SCHE_SRC_OSTD_PORT0_CTRL_REG_DEFAULT);
            break;
        case READ_WRITE_NOT_MERGE:
            reg_val = (cfg->otsd_lvl[0] << 0U) | (OTSD_ENABLE << 8U) | (cfg->otsd_lvl[1] << 9U) | (OTSD_ENABLE << 17U);
            set_g_stored_reg_val(SCHE_SRC_OSTD_PORT0_CTRL_REG_IDX, reg_val);
            break;
        default:
            pr_err("[hclge_plf]:Invalid otsd_mode %d\n", cfg->otsd_mode);
            return -EINVAL;
    }

    if (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state) || test_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state) == 0) {
        pr_warn("[hclge_plf]: try to read reg while resetting!\n");
        return -EBUSY;
    }

    set_bit(HCLGE_STATE_QOS_CFG_CHANGED, &hdev->state);
    return 0;
}

STATIC int hclge_plf_nic_get_otsd_cfg(int devid, struct qos_otsd_config_type *cfg)
{
#define CFG0_SRC_RD_OSTD_LVL_OFF 9

    struct hclge_plf_dev *hdev;
    u32 reg_val;

    if (devid != 0) {
        pr_err("[hclge_plf]:Invalid devid %d\n", devid);
        return -EINVAL;
    }

    if (cfg == NULL) {
        pr_err("[hclge_plf]:cfg is null!\n");
        return -EINVAL;
    }

    if (cfg->master != MASTER_NIC) {
        pr_err("[hclge_plf]:Invalid master %d\n", cfg->master);
        return -EINVAL;
    }

    hdev = hclge_get_first_hclge_plf_dev();
    if (hdev == NULL) {
        pr_err("hns3: hclge get hclge plf dev fail.\n");
        return -EIO;
    }

    if (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state) || test_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state) == 0) {
        pr_err("[hclge_plf]: try to read reg while resetting!\n");
        return -EBUSY;
    }

    reg_val = readl(hdev->hw.sche_base + SCHE_SRC_OSTD_PORT0_CTRL_REG_OFFSET);

    cfg->otsd_lvl[0] = (reg_val & 0x1FE00) >> CFG0_SRC_RD_OSTD_LVL_OFF;
    cfg->otsd_lvl[1] = reg_val & 0xFF;

    return 0;
}

/* dynamic */
STATIC int hclge_plf_nic_set_allow_cfg(int devid, const struct qos_allow_config_type *cfg)
{
    struct hclge_plf_dev *hdev;
    u32 reg_val;
    u32 i;

    if (devid != 0) {
        pr_err("[hclge_plf]:Invalid devid %d\n", devid);
        return -EINVAL;
    }

    if (cfg == NULL) {
        pr_err("[hclge_plf]:cfg is null!\n");
        return -EINVAL;
    }

    if (cfg->master != MASTER_NIC) {
        pr_err("[hclge_plf]: Invalid master %d\n", cfg->master);
        return -EINVAL;
    }

    hdev = hclge_get_first_hclge_plf_dev();
    if (hdev == NULL) {
        pr_err("hns3: hclge get hclge plf dev fail.\n");
        return -EIO;
    }

    switch (cfg->qos_allow_mode) {
        /* 0 -- disable bp, 1 -- produce bp, 2 -- response bp */
        case DISABLE_BP:
            set_g_stored_reg_val(SCHE_AR_OSTD_PORT0_CTRL_REG_IDX, SCHE_AR_OSTD_PORT0_CTRL_REG_DEFAULT);
            set_g_stored_reg_val(SCHE_AW_OSTD_PORT0_CTRL_REG_IDX, SCHE_AW_OSTD_PORT0_CTRL_REG_DEFAULT);
            break;
        case RESPONSE_BP:
            /* lvl1 > lvl2 > lvl3 */
            if (cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_0] <= cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_1] ||
                cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_1] <= cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_2]) {
                pr_err("[hclge_plf]:Invalid qos allow level %d %d %d\n",
                    cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_0],
                    cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_1],
                    cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_2]);
                return -EINVAL;
            }

            reg_val = (cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_0] << 0U) |
                      (cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_1] << 8U) |
                      (cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_2] << 16U);
            reg_val = reg_val | (0x2 << 25U);

            if (cfg->qos_allow_ctrl == QOS_ALLOW_CTRL_ALL || cfg->qos_allow_ctrl == QOS_ALLOW_CTRL_READ) {
                set_g_stored_reg_val(SCHE_AR_OSTD_PORT0_CTRL_REG_IDX, reg_val);
            }
            if (cfg->qos_allow_ctrl == QOS_ALLOW_CTRL_ALL || cfg->qos_allow_ctrl == QOS_ALLOW_CTRL_WRITE) {
                set_g_stored_reg_val(SCHE_AW_OSTD_PORT0_CTRL_REG_IDX, reg_val);
            }
            break;
        default:
            pr_err("[hclge_plf]: Invalid qos_allow_mode %d\n", cfg->qos_allow_mode);
            return -EINVAL;
    }

    if (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state) || test_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state) == 0) {
        pr_warn("[hclge_plf]: try to read reg while resetting!\n");
        return -EBUSY;
    }

    set_bit(HCLGE_STATE_QOS_CFG_CHANGED, &hdev->state);
    return 0;
}

STATIC int hclge_plf_nic_get_allow_cfg(int devid, struct qos_allow_config_type *cfg)
{
    struct hclge_plf_dev *hdev;
    u32 reg_val;

    if (devid != 0) {
        pr_err("[hclge_plf]: Invalid devid %d\n", devid);
        return -EINVAL;
    }

    if (cfg == NULL) {
        pr_err("[hclge_plf]: cfg is null!\n");
        return -EINVAL;
    }

    if (cfg->master != MASTER_NIC) {
        pr_err("[hclge_plf]: Invalid master %d\n", cfg->master);
        return -EINVAL;
    }

    hdev = hclge_get_first_hclge_plf_dev();
    if (hdev == NULL) {
        pr_err("hns3: hclge get hclge plf dev fail.\n");
        return -EIO;
    }

    if (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state) || test_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state) == 0) {
        pr_err("[hclge_plf]: try to read reg while resetting!\n");
        return -EBUSY;
    }

    switch (cfg->qos_allow_ctrl) {
        case QOS_ALLOW_CTRL_READ:
            reg_val = readl(hdev->hw.sche_base + SCHE_AR_OSTD_PORT0_CTRL_REG_OFFSET);
            cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_0] = reg_val & 0xFF;
            cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_1] = (reg_val >> 8U) & 0xFF;
            cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_2] = (reg_val >> 16U) & 0xFF;
            break;
        case QOS_ALLOW_CTRL_WRITE:
            reg_val = readl(hdev->hw.sche_base + SCHE_AW_OSTD_PORT0_CTRL_REG_OFFSET);
            cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_0] = reg_val & 0xFF;
            cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_1] = (reg_val >> 8U) & 0xFF;
            cfg->qos_allow_lvl[QOS_ALLOW_LVL_IDX_2] = (reg_val >> 16U) & 0xFF;
            break;
        default:
            pr_err("[hclge_plf]: Invalid qos_allow_ctrl %d\n", cfg->qos_allow_ctrl);
            return -EINVAL;
    }

    return 0;
}

STATIC int nic_qos_node_register(void)
{
    int ret;

    ret = strcpy_s(g_node_nic.name, QOS_NODE_NAME_MAX_LEN, "NIC_QOS");
    if (ret != 0) {
        pr_err("[hclge_plf]: Call strcpy_s failed. (ret=%d)\n", ret);
        return ret;
    }
    g_node_nic.cfg.type = MASTER_NIC;
    g_node_nic.set = hclge_plf_nic_set_qos_cfg;
    g_node_nic.get = hclge_plf_nic_get_qos_cfg;
    g_node_nic.set_allow = hclge_plf_nic_set_allow_cfg;
    g_node_nic.get_allow = hclge_plf_nic_get_allow_cfg;
    g_node_nic.set_otsd = hclge_plf_nic_set_otsd_cfg;
    g_node_nic.get_otsd = hclge_plf_nic_get_otsd_cfg;

    ret = hal_kernel_qos_node_register(&g_node_nic);
    if (ret != 0) {
        pr_err("[hclge_plf]: nic qos node resigster failed, ret = %d\n", ret);
        return ret;
    }

    return 0;
}

STATIC int hclge_plf_tpu_set_otsd_cfg(int devid, const struct qos_otsd_config_type *cfg)
{
    struct hclge_plf_dev *hdev;
    u32 reg_val;

    if (devid != 0) {
        pr_err("[hclge_plf]: Invalid devid %d\n", devid);
        return -EINVAL;
    }

    if (cfg == NULL) {
        pr_err("[hclge_plf]: cfg is null!\n");
        return -EINVAL;
    }

    if (cfg->master != MASTER_TPU) {
        pr_err("[hclge_plf]: Invalid master %d\n", cfg->master);
        return -EINVAL;
    }

    hdev = hclge_get_first_hclge_plf_dev();
    if (hdev == NULL) {
        pr_err("hns3: hclge get hclge plf dev fail.\n");
        return -EIO;
    }

    switch (cfg->otsd_mode) {
        /* 0 -- disable otsd limit, 1 -- read & write merge, 2 -- read & write not merge */
        case DISABLE_OTSD_LIMIT:
            set_g_stored_reg_val(PPE_TPU_TPU_OUTSTAND_CTRL_REG_IDX, PPE_TPU_TPU_OUTSTAND_CTRL_REG_DEFAULT);
            break;
        case READ_WRITE_MERGE:
            reg_val = cfg->otsd_lvl[0];

            /* [0x8,0x10] */
            if (reg_val < 0x8 || reg_val > 0x10) {
                pr_err("[hclge_plf]: Invalid ostd_lvl %d\n", reg_val);
                return -EINVAL;
            }

            set_g_stored_reg_val(PPE_TPU_TPU_OUTSTAND_CTRL_REG_IDX, reg_val & 0x7F);
            break;
        default:
            pr_err("[hclge_plf]: Invalid otsd_mode %d\n", cfg->master);
            return -EINVAL;
    }

    if (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state) || test_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state) == 0) {
        pr_warn("[hclge_plf]: try to read reg while resetting!\n");
        return -EBUSY;
    }

    set_bit(HCLGE_STATE_QOS_CFG_CHANGED, &hdev->state);
    return 0;
}

STATIC int hclge_plf_tpu_get_otsd_cfg(int devid, struct qos_otsd_config_type *cfg)
{
    struct hclge_plf_dev *hdev;
    u32 reg_addr;
    u32 reg_val;

    if (devid != 0) {
        pr_err("[hclge_plf]: Invalid devid %d\n", devid);
        return -EINVAL;
    }

    if (cfg == NULL) {
        pr_err("[hclge_plf]: cfg is null!\n");
        return -EINVAL;
    }

    if (cfg->master != MASTER_TPU) {
        pr_err("[hclge_plf]: Invalid master %d\n", cfg->master);
        return -EINVAL;
    }

    hdev = hclge_get_first_hclge_plf_dev();
    if (hdev == NULL) {
        pr_err("hns3: hclge get hclge plf dev fail.\n");
        return -EIO;
    }

    if (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state) || test_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state) == 0) {
        pr_err("[hclge_plf]: try to read reg while resetting!\n");
        return -EBUSY;
    }

    reg_addr = TPU_REG_BASE + PPE_TPU_TPU_OUTSTAND_CTRL_REG;
    reg_val = hclge_read_dev(&hdev->hw, reg_addr);

    cfg->otsd_lvl[0] = reg_val & 0x7F;
    return 0;
}

STATIC int tpu_qos_node_register(void)
{
    int ret;

    ret = strcpy_s(g_node_tpu.name, QOS_NODE_NAME_MAX_LEN, "TPU_QOS");
    if (ret != 0) {
        pr_err("[hclge_plf]: Call strcpy_s failed, ret = %d.\n", ret);
        return -EINVAL;
    }
    g_node_tpu.cfg.type = MASTER_TPU;
    g_node_tpu.set_otsd = hclge_plf_tpu_set_otsd_cfg;
    g_node_tpu.get_otsd = hclge_plf_tpu_get_otsd_cfg;
    g_node_tpu.set_allow = NULL;
    g_node_tpu.get_allow = NULL;
    g_node_tpu.set = NULL;
    g_node_tpu.get = NULL;

    ret = hal_kernel_qos_node_register(&g_node_tpu);
    if (ret != 0) {
        pr_err("[hclge_plf]: tpu qos node resigster failed, ret = %d\n", ret);
        return -EINVAL;
    }

    return 0;
}

STATIC int hclge_plf_rpu_set_otsd_cfg(int devid, const struct qos_otsd_config_type *cfg)
{
    struct hclge_plf_dev *hdev;
    u32 reg_val;

    if (devid != 0) {
        pr_err("[hclge_plf]: Invalid devid %d\n", devid);
        return -EINVAL;
    }

    if (cfg == NULL) {
        pr_err("[hclge_plf]: cfg is null!\n");
        return -EINVAL;
    }

    if (cfg->master != MASTER_RPU) {
        pr_err("[hclge_plf]: Invalid master %d\n", cfg->master);
        return -EINVAL;
    }

    hdev = hclge_get_first_hclge_plf_dev();
    if (hdev == NULL) {
        pr_err("hns3: hclge get hclge plf dev fail.\n");
        return -EIO;
    }

    switch (cfg->otsd_mode) {
        /* 0 -- disable otsd limit, 1 -- read & write merge, 2 -- read & write not merge */
        case 0:
            set_g_stored_reg_val(PPE_RPU_RPU_WR_AXI_FIFO_THD1_REG_IDX, PPE_RPU_RPU_WR_AXI_FIFO_THD1_REG_DEFAULT);
            break;
        case 1:
            reg_val = cfg->otsd_lvl[0];

            /* [0x1,0xe] */
            if (reg_val < 0x1 || reg_val > 0xe) {
                pr_err("[hclge_plf]: Invalid ostd_lvl %d\n", reg_val);
                return -EINVAL;
            }

            set_g_stored_reg_val(PPE_RPU_RPU_WR_AXI_FIFO_THD1_REG_IDX, (0x2D << 8U) | (reg_val & 0x7F));
            break;
        default:
            pr_err("[hclge_plf]: Invalid otsd_mode %d\n", cfg->master);
            return -EINVAL;
    }

    if (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state) || test_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state) == 0) {
        pr_warn("[hclge_plf]: try to read reg while resetting!\n");
        return -EBUSY;
    }

    set_bit(HCLGE_STATE_QOS_CFG_CHANGED, &hdev->state);
    return 0;
}

STATIC int hclge_plf_rpu_get_otsd_cfg(int devid, struct qos_otsd_config_type *cfg)
{
    struct hclge_plf_dev *hdev;
    u32 reg_addr;
    u32 reg_val;

    if (devid != 0) {
        pr_err("[hclge_plf]: Invalid devid %d\n", devid);
        return -EINVAL;
    }

    if (cfg == NULL) {
        pr_err("[hclge_plf]: cfg is null!\n");
        return -EINVAL;
    }

    if (cfg->master != MASTER_RPU) {
        pr_err("[hclge_plf]:Invalid master %d\n", cfg->master);
        return -EINVAL;
    }

    hdev = hclge_get_first_hclge_plf_dev();
    if (hdev == NULL) {
        pr_err("hns3: hclge get hclge plf dev fail.\n");
        return -EIO;
    }

    if (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state) || test_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state) == 0) {
        pr_err("[hclge_plf]: try to read reg while resetting!\n");
        return -EBUSY;
    }

    reg_addr = RPU_REG_BASE + PPE_RPU_RPU_WR_AXI_FIFO_THD1_REG;
    reg_val = hclge_read_dev(&hdev->hw, reg_addr);

    cfg->otsd_lvl[0] = reg_val & 0x7F;
    return 0;
}

STATIC int rpu_qos_node_register(void)
{
    int ret;

    ret = strcpy_s(g_node_rpu.name, QOS_NODE_NAME_MAX_LEN, "RPU_QOS");
    if (ret != 0) {
        pr_err("[hclge_plf]: Call strcpy_s failed, ret = %d.\n", ret);
        return ret;
    }
    g_node_rpu.cfg.type = MASTER_RPU;
    g_node_rpu.set_otsd = hclge_plf_rpu_set_otsd_cfg;
    g_node_rpu.get_otsd = hclge_plf_rpu_get_otsd_cfg;
    g_node_rpu.set_allow = NULL;
    g_node_rpu.get_allow = NULL;
    g_node_rpu.set = NULL;
    g_node_rpu.get = NULL;

    ret = hal_kernel_qos_node_register(&g_node_rpu);
    if (ret != 0) {
        pr_err("[hclge_plf]: rpu qos node resigster failed, ret = %d\n", ret);
        return ret;
    }

    return 0;
}

STATIC void qos_node_unregister(struct qos_master_node *node)
{
    (void)hal_kernel_qos_node_unregister(node);
    node->cfg.type = MASTER_INVALID;
    node->set = NULL;
    node->get = NULL;
    node->set_allow = NULL;
    node->get_allow = NULL;
    node->set_otsd = NULL;
    node->get_otsd = NULL;
}

int hclge_plf_qos_node_register(void)
{
    int ret;

    ret = nic_qos_node_register();
    if (ret != 0) {
        pr_err("[hclge_plf]: failed to register NIC_QOS node, ret = %d\n", ret);
        return ret;
    }

    ret = tpu_qos_node_register();
    if (ret != 0) {
        pr_err("[hclge_plf]: failed to register TPU_QOS node, ret = %d\n", ret);
        goto err_unregister_nic_qos_nod;
    }

    ret = rpu_qos_node_register();
    if (ret != 0) {
        pr_err("[hclge_plf]: failed to register RPU_QOS node, ret = %d\n", ret);
        goto err_unregister_tpu_qos_nod;
    }

    return 0;

err_unregister_tpu_qos_nod:
    qos_node_unregister(&g_node_tpu);
err_unregister_nic_qos_nod:
    qos_node_unregister(&g_node_nic);
    return ret;
}

void hclge_plf_qos_node_unregister(void)
{
    qos_node_unregister(&g_node_nic);
    qos_node_unregister(&g_node_rpu);
    qos_node_unregister(&g_node_tpu);
}

void hclge_plf_update_qos_node(struct hclge_plf_dev *hdev)
{
    u32 reg_addr;
    u32 reg_val;
    int i;

    if (hdev->id != hclge_plf_get_first_probe_id()) {
        return;
    }

    if (test_bit(HCLGE_STATE_RST_HANDLING, &hdev->state) || test_bit(HCLGE_STATE_NIC_REGISTERED, &hdev->state) == 0) {
        return;
    }

    if (!test_and_clear_bit(HCLGE_STATE_QOS_CFG_CHANGED, &hdev->state)) {
        return;
    }

    reg_addr = TPU_REG_BASE + PPE_TPU_TPU_OUTSTAND_CTRL_REG;
    reg_val = get_g_stored_reg_val(PPE_TPU_TPU_OUTSTAND_CTRL_REG_IDX);
    hclge_write_dev(&hdev->hw, reg_addr, reg_val);

    reg_addr = RPU_REG_BASE + PPE_RPU_RPU_WR_AXI_FIFO_THD1_REG;
    reg_val = get_g_stored_reg_val(PPE_RPU_RPU_WR_AXI_FIFO_THD1_REG_IDX);
    hclge_write_dev(&hdev->hw, reg_addr, reg_val);

    reg_val = get_g_stored_reg_val(SCHE_SRC_OSTD_PORT0_CTRL_REG_IDX);
    writel(reg_val, hdev->hw.sche_base + SCHE_SRC_OSTD_PORT0_CTRL_REG_OFFSET);

    reg_addr = RCB_COM_REG_BASE + PPE_RCB_COM_RCB_AXQOS_CFG_REG;
    reg_val = get_g_stored_reg_val(PPE_RCB_COM_RCB_AXQOS_CFG_REG_IDX);
    hclge_write_dev(&hdev->hw, reg_addr, reg_val);

    reg_addr = TPU_REG_BASE + PPE_TPU_TPU_CFG_BUS_QOS_REG;
    reg_val = get_g_stored_reg_val(PPE_TPU_TPU_CFG_BUS_QOS_REG_IDX);
    hclge_write_dev(&hdev->hw, reg_addr, reg_val);

    reg_addr = RPU_REG_BASE + PPE_RPU_RPU_AXQOS_CFG_REG;
    reg_val = get_g_stored_reg_val(PPE_RPU_RPU_AXQOS_CFG_REG_IDX);
    hclge_write_dev(&hdev->hw, reg_addr, reg_val);

    reg_val = get_g_stored_reg_val(PPE_TPU_TPU_DDR_SMMU_USER1_0_REG_IDX);
    for (i = 0; i < MAX_PORT_NUM; i++) {
        reg_addr = TPU_REG_BASE + PPE_TPU_TPU_DDR_SMMU_USER1_0_REG + i * TPU_PORT_OFFSET;
        hclge_write_dev(&hdev->hw, reg_addr, reg_val);
    }

    reg_val = get_g_stored_reg_val(SCHE_AR_OSTD_PORT0_CTRL_REG_IDX);
    writel(reg_val, hdev->hw.sche_base + SCHE_AR_OSTD_PORT0_CTRL_REG_OFFSET);

    reg_val = get_g_stored_reg_val(SCHE_AW_OSTD_PORT0_CTRL_REG_IDX);
    writel(reg_val, hdev->hw.sche_base + SCHE_AW_OSTD_PORT0_CTRL_REG_OFFSET);
}

#endif