/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: platform debugfs
 * Author: huawei
 * Create: 2022-09-15
 */

#include <linux/device.h>

#include "hclge_plf_debugfs.h"

static struct hclge_plf_dbg_reg_type_info hclge_plf_dbg_reg_info[] = {
    {.cmd = HNAE3_DBG_CMD_REG_SSU,
     .dfx_msg = &hclge_plf_dbg_ssu_reg[0],
     .msg_num = ARRAY_SIZE(hclge_plf_dbg_ssu_reg)},
    {.cmd = HNAE3_DBG_CMD_REG_IGU_EGU,
     .dfx_msg = &hclge_plf_dbg_igu_egu_reg[0],
     .msg_num = ARRAY_SIZE(hclge_plf_dbg_igu_egu_reg)},
    {.cmd = HNAE3_DBG_CMD_REG_RPU,
     .dfx_msg = &hclge_plf_dbg_rpu_reg[0],
     .msg_num = ARRAY_SIZE(hclge_plf_dbg_rpu_reg)},
    {.cmd = HNAE3_DBG_CMD_REG_PPP,
     .dfx_msg = &hclge_plf_dbg_ppp_reg[0],
     .msg_num = ARRAY_SIZE(hclge_plf_dbg_ppp_reg)},
    {.cmd = HNAE3_DBG_CMD_REG_RCB,
     .dfx_msg = &hclge_plf_dbg_rcb_reg[0],
     .msg_num = ARRAY_SIZE(hclge_plf_dbg_rcb_reg)},
    {.cmd = HNAE3_DBG_CMD_REG_TQP,
     .dfx_msg = &hclge_plf_dbg_tqp_reg[0],
     .msg_num = ARRAY_SIZE(hclge_plf_dbg_tqp_reg)},
    {.cmd = HNAE3_DBG_CMD_REG_PA,
     .dfx_msg = &hclge_plf_dbg_pa_reg[0],
     .msg_num = ARRAY_SIZE(hclge_plf_dbg_pa_reg)},
    {.cmd = HNAE3_DBG_CMD_REG_TPU,
     .dfx_msg = &hclge_plf_dbg_tpu_reg[0],
     .msg_num = ARRAY_SIZE(hclge_plf_dbg_tpu_reg)},
    {.cmd = HNAE3_DBG_CMD_REG_MAC,
     .dfx_msg = &hclge_plf_dbg_mac_reg[0],
     .msg_num = ARRAY_SIZE(hclge_plf_dbg_mac_reg)},
};

static void hclge_plf_dbg_dump_reg_tqp(struct hclge_plf_dev *hdev,
                                       const struct hclge_plf_dbg_reg_type_info *reg_info,
                                       char *buf, int len, int *pos)
{
    const struct hclge_plf_dbg_dfx_message *dfx_message = reg_info->dfx_msg;
    u32 index, i, cnt;
    u32 reg_val = 0;

    for (i = 0, cnt = 0; i < reg_info->msg_num; i++, dfx_message++) {
        *pos += scnprintf(buf + *pos, (size_t)((u32)(len - *pos)), "item%u = %s\n", cnt++, dfx_message->message);
    }

    for (i = 0; i < cnt; i++) {
        *pos += scnprintf(buf + *pos, (size_t)((u32)(len - *pos)), "item%u\t", i);
    }

    *pos += scnprintf(buf + *pos, (size_t)((u32)(len - *pos)), "\n");

    for (index = 0; index < hdev->vport[0].alloc_tqps; index++) {
        dfx_message = reg_info->dfx_msg;

        for (i = 0; i < reg_info->msg_num; i++, dfx_message++) {
            if (!strcmp("q_num", dfx_message->message)) {
                reg_val = hdev->id;
            } else {
                reg_val =
                    hclge_read_dev(&hdev->hw, (dfx_message->reg + (index + hdev->tqp_offset) * dfx_message->offset));
            }
            *pos += scnprintf(buf + *pos, (size_t)((u32)(len - *pos)), "%#x\t", reg_val);
        }
        *pos += scnprintf(buf + *pos, (size_t)((u32)(len - *pos)), "\n");
    }
}

static void hclge_plf_dbg_dump_reg_common(struct hclge_plf_dev *hdev,
                                          const struct hclge_plf_dbg_reg_type_info *reg_info,
                                          char *buf, int len, int *pos)
{
    const struct hclge_plf_dbg_dfx_message *dfx_message = reg_info->dfx_msg;
    u32 reg_val = 0;
    u32 i;

    for (i = 0; i < reg_info->msg_num; i++, dfx_message++) {
        if (!dfx_message->flag) {
            continue;
        }

        reg_val = hclge_read_dev(&hdev->hw, (dfx_message->reg + hdev->id * dfx_message->offset));
        *pos += scnprintf(buf + *pos, (size_t)((u32)(len - *pos)), "%s: %#x\n", dfx_message->message, reg_val);
    }
}

static void hclge_plf_dbg_dump_reg_cmd(struct hclge_plf_dev *hdev, enum hnae3_dbg_cmd cmd, char *buf, int len)
{
    const struct hclge_plf_dbg_reg_type_info *reg_info;
    int pos = 0;
    u32 i;

    if (len < 0) {
        dev_err(&hdev->pdev->dev, "dump reg invalid paramter, len = %d.\n", len);
        return;
    }

    for (i = 0; i < ARRAY_SIZE(hclge_plf_dbg_reg_info); i++) {
        reg_info = &hclge_plf_dbg_reg_info[i];
        if (cmd == reg_info->cmd) {
            if (cmd == HNAE3_DBG_CMD_REG_TQP) {
                hclge_plf_dbg_dump_reg_tqp(hdev, reg_info, buf, len, &pos);
            } else {
                hclge_plf_dbg_dump_reg_common(hdev, reg_info, buf, len, &pos);
            }
        }
    }
}

static void hclge_plf_dbg_dump_interrupt(struct hclge_plf_dev *hdev, char *buf, int len)
{
    int pos = 0;

    if (len < 0) {
        dev_err(&hdev->pdev->dev, "dump interrupt invalid paramter, len = %d.\n", len);
        return;
    }

    pos += scnprintf(buf + pos, (size_t)((u32)(len - pos)), "num_nic_msi: %d\n", hdev->num_nic_msi);
    pos += scnprintf(buf + pos, (size_t)((u32)(len - pos)), "num_msi_used: %d\n", hdev->num_msi_used);
    pos += scnprintf(buf + pos, (size_t)((u32)(len - pos)), "num_msi_left: %d\n", hdev->num_msi_left);
}

static void hclge_plf_dbg_dump_dcb(struct hclge_plf_dev *hdev, char *buf, int len)
{
    u32 reg_val = 0;
    int pos = 0;

    if (len < 0) {
        dev_err(&hdev->pdev->dev, "dump dcb invalid paramter, len = %d.\n", len);
        return;
    }

    pos += scnprintf(buf + pos, (size_t)((u32)(len - pos)), "TC_NUM: %d\n", hdev->tm_info.num_tc);

    reg_val = hclge_read_dev(&hdev->hw, (TX_PA_REG_BASE + PA_PRI_MAP_TC + hdev->id * 0x30)); // offset : 0x30
    pos += scnprintf(buf + pos, (size_t)((u32)(len - pos)), "TX_PRI_MAP_TC_CFG: %#x\n", reg_val);
    reg_val = hclge_read_dev(&hdev->hw, (RX_PA_REG_BASE + PA_PRI_MAP_TC + hdev->id * 0x30)); // offset : 0x30
    pos += scnprintf(buf + pos, (size_t)((u32)(len - pos)), "RX_PRI_MAP_TC_CFG: %#x\n", reg_val);
}

static void hclge_plf_dbg_dump_rst_info(struct hclge_plf_dev *hdev, char *buf, int len)
{
    int pos = 0;

    if (len < 0) {
        dev_err(&hdev->pdev->dev, "dump reset info invalid paramter, len = %d.\n", len);
        return;
    }

    pos += scnprintf(buf + pos, (size_t)(len - pos), "PF reset count: %u\n", hdev->rst_stats.pf_rst_cnt);
    pos += scnprintf(buf + pos, (size_t)(len - pos), "GLOBAL reset count: %u\n", hdev->rst_stats.global_rst_cnt);
    pos += scnprintf(buf + pos, (size_t)(len - pos), "reset done count: %u\n", hdev->rst_stats.reset_done_cnt);
    pos += scnprintf(buf + pos, (size_t)(len - pos), "reset count: %u\n", hdev->rst_stats.reset_cnt);
    pos += scnprintf(buf + pos, (size_t)(len - pos), "reset fail count: %u\n", hdev->rst_stats.reset_fail_cnt);
}

static const struct hclge_plf_dbg_func hclge_plf_dbg_cmd_func[] = {
    {
        .cmd = HNAE3_DBG_CMD_REG_SSU,
        .dbg_dump_reg = hclge_plf_dbg_dump_reg_cmd,
    },
    {
        .cmd = HNAE3_DBG_CMD_REG_IGU_EGU,
        .dbg_dump_reg = hclge_plf_dbg_dump_reg_cmd,
    },
    {
        .cmd = HNAE3_DBG_CMD_REG_RPU,
        .dbg_dump_reg = hclge_plf_dbg_dump_reg_cmd,
    },
    {
        .cmd = HNAE3_DBG_CMD_REG_PPP,
        .dbg_dump_reg = hclge_plf_dbg_dump_reg_cmd,
    },
    {
        .cmd = HNAE3_DBG_CMD_REG_RCB,
        .dbg_dump_reg = hclge_plf_dbg_dump_reg_cmd,
    },
    {
        .cmd = HNAE3_DBG_CMD_REG_TQP,
        .dbg_dump_reg = hclge_plf_dbg_dump_reg_cmd,
    },
    {
        .cmd = HNAE3_DBG_CMD_REG_PA,
        .dbg_dump_reg = hclge_plf_dbg_dump_reg_cmd,
    },
    {
        .cmd = HNAE3_DBG_CMD_REG_TPU,
        .dbg_dump_reg = hclge_plf_dbg_dump_reg_cmd,
    },
    {
        .cmd = HNAE3_DBG_CMD_REG_MAC,
        .dbg_dump_reg = hclge_plf_dbg_dump_reg_cmd,
    },
    {
        .cmd = HNAE3_DBG_CMD_REG_DCB,
        .dbg_dump = hclge_plf_dbg_dump_dcb,
    },
    {
        .cmd = HNAE3_DBG_CMD_INTERRUPT_INFO,
        .dbg_dump = hclge_plf_dbg_dump_interrupt,
    },
    {
        .cmd = HNAE3_DBG_CMD_RESET_INFO,
        .dbg_dump = hclge_plf_dbg_dump_rst_info,
    },
};

int hclge_plf_dbg_read_cmd(struct hnae3_handle *handle, enum hnae3_dbg_cmd cmd, char *buf, int len)
{
    struct hclge_plf_vport *vport = hclge_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    u32 i;

    for (i = 0; i < ARRAY_SIZE(hclge_plf_dbg_cmd_func); i++) {
        if (cmd == hclge_plf_dbg_cmd_func[i].cmd) {
            break;
        }
    }

    if (i >= ARRAY_SIZE(hclge_plf_dbg_cmd_func)) {
        dev_err(&hdev->pdev->dev, "invalid command(%d)\n", cmd);
        return -EINVAL;
    }

    if (hclge_plf_dbg_cmd_func[i].dbg_dump)
        hclge_plf_dbg_cmd_func[i].dbg_dump(hdev, buf, len);
    else
        hclge_plf_dbg_cmd_func[i].dbg_dump_reg(hdev, cmd, buf, len);

    return 0;
}
