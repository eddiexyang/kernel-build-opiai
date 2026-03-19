/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: RAS
 * Author: huawei
 * Create: 2022-12-15
 */

#include "reg_ssu_offset.h"
#include "reg_ppp_offset.h"
#include "reg_rcb_com_offset.h"
#include "reg_tpu_offset.h"
#include "reg_rpu_offset.h"
#include "igu_egu_cfg_reg_offset.h"
#include "xxvge_common_reg_offset.h"
#include "reg_top_offset.h"
#include "hclge_plf_main.h"
#include "dfm_dev_register.h"
#include "dfm_report.h"
#include "hclge_plf_err.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

STATIC const struct hclge_plf_hw_error g_hclge_plf_ssu_err_int[] = {
    {
        .int_msk = BIT(0),
        .msg = "mem_ecc_sbit_int_status",
        .type_id = MEM_SBECC_ERROR,
        .reset_level = HNAE3_NONE_RESET
    }, {
        .int_msk = BIT(1),
        .msg = "mem_ecc_mbit_int_status",
        .type_id = MEM_MBECC_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(2),
        .msg = "fifo_rd_int_status",
        .type_id = FIFO_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(3),
        .msg = "fifo_wr_int_status",
        .type_id = FIFO_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(4),
        .msg = "host_pkt_sof_mismatch_status",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(5),
        .msg = "host_pkt_eof_mismatch_status",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(6),
        .msg = "host_pkt_key_mismatch_status",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(7),
        .msg = "mac_pkt_sof_mismatch_status",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(8),
        .msg = "mac_pkt_eof_mismatch_status",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(9),
        .msg = "mac_pkt_key_mismatch_status",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(10),
        .msg = "rpu_credit_err_int_status",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(11),
        .msg = "bmp_rsc_return_int_status",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(14),
        .msg = "qm_free_rsc_int_status",
        .type_id = BUFFER_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }
};

STATIC const struct hclge_plf_hw_error g_hclge_plf_rcb_mpf_err_int[] = {
    {
        .int_msk = BIT(0),
        .msg = "rcb_ecc_merr_int_st",
        .type_id = MEM_MBECC_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(1),
        .msg = "rcb_ecc_serr_int_st",
        .type_id = MEM_SBECC_ERROR,
        .reset_level = HNAE3_NONE_RESET
    }, {
        .int_msk = BIT(2),
        .msg = "rd_bus_err_int_st",
        .type_id = FIFO_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(3),
        .msg = "wr_bus_err_int_st",
        .type_id = FIFO_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(4),
        .msg = "reg_search_miss_int_st",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_NONE_RESET
    }, {
        .int_msk = BIT(5),
        .msg = "rx_q_search_miss_int_st",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_NONE_RESET
    }
};

STATIC const struct hclge_plf_hw_error g_hclge_plf_rcb_pf_err_int[] = {
    {
        .int_msk = BIT(0),
        .msg = "over_8bd_no_fe_int_st",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(1),
        .msg = "tso_mss_cmp_min_err_int_st",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(2),
        .msg = "tso_mss_cmp_max_err_int_st",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(3),
        .msg = "tx_rd_fbd_poison_int_st",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(4),
        .msg = "rx_rd_ebd_poison_int_st",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }
};

STATIC const struct hclge_plf_hw_error g_hclge_plf_tpu_err_int[] = {
    {
        .int_msk = BIT(0),
        .msg = "mem_ecc_1bit_int_sts",
        .type_id = MEM_SBECC_ERROR,
        .reset_level = HNAE3_NONE_RESET
    }, {
        .int_msk = BIT(1),
        .msg = "mem_ecc_2bit_int_sts",
        .type_id = MEM_MBECC_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(2),
        .msg = "fifo_rd_int_sts",
        .type_id = FIFO_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(3),
        .msg = "fifo_wr_int_sts",
        .type_id = FIFO_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(5),
        .msg = "axi_err_int_sts",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }
};

STATIC const struct hclge_plf_hw_error g_hclge_plf_rpu_pf_err_int[] = {
    {
        .int_msk = BIT(0),
        .msg = "mem_ecc_sbit_int_sts",
        .type_id = MEM_SBECC_ERROR,
        .reset_level = HNAE3_NONE_RESET
    }, {
        .int_msk = BIT(1),
        .msg = "mem_ecc_mbit_int_sts",
        .type_id = MEM_MBECC_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(2),
        .msg = "fifo_rd_int_sts",
        .type_id = FIFO_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(3),
        .msg = "fifo_wr_int_sts",
        .type_id = FIFO_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(4),
        .msg = "fsm_hbeat_int_sts",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(5),
        .msg = "wr_axi_err_int_sts",
        .type_id = ABNORMAL_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }
};

STATIC const struct hclge_plf_hw_error g_hclge_plf_rpu_mode_err_int[] = {
    {
        .int_msk = BIT(0),
        .msg = "buf_aban_int_sts",
        .type_id = BUFFER_ERROR,
        .reset_level = HNAE3_NONE_RESET
    }, {
        .int_msk = BIT(1),
        .msg = "buf_empty_int_sts",
        .type_id = BUFFER_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(2),
        .msg = "buf_len_err_int_sts",
        .type_id = BUFFER_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }
};

STATIC const struct hclge_plf_hw_error g_hclge_plf_ppp_fifo_err_int[] = {
    {
        .int_msk = BIT(0),
        .msg = "fifo_uf_rint_sts",
        .type_id = FIFO_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(1),
        .msg = "fifo_of_rint_sts",
        .type_id = FIFO_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }
};

STATIC const struct hclge_plf_hw_error g_hclge_plf_ppp_mem_sbit_err_int[] = {
    {
        .int_msk = BIT(12),
        .msg = "fd_tcam_ecc_mem_sb_rint_sts",
        .type_id = MEM_SBECC_ERROR,
        .reset_level = HNAE3_NONE_RESET
    }
};

STATIC const struct hclge_plf_hw_error g_hclge_plf_ppp_mem_mbit_err_int[] = {
    {
        .int_msk = BIT(12),
        .msg = "fd_tcam_ecc_mem_mb_rint_sts",
        .type_id = MEM_MBECC_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }
};

STATIC const struct hclge_plf_hw_error g_hclge_plf_igu_egu_err_int[] = {
    {
        .int_msk = BIT(0),
        .msg = "rx_buf_overflow_int_sts",
        .type_id = BUFFER_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(1),
        .msg = "rx_stp_fifo_overflow_int_sts",
        .type_id = FIFO_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(2),
        .msg = "rx_stp_fifo_underflow_int_sts",
        .type_id = FIFO_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(3),
        .msg = "tx_buf_overflow_int_sts",
        .type_id = BUFFER_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(4),
        .msg = "tx_buf_underun_int_sts",
        .type_id = BUFFER_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }
};

STATIC const struct hclge_plf_hw_error g_hclge_plf_xxvge_err_int[] = {
    {
        .int_msk = BIT(0),
        .msg = "egu_lge_afifo_ovf_int_sts",
        .type_id = FIFO_ERROR,
        .reset_level = HNAE3_GLOBAL_RESET
    }, {
        .int_msk = BIT(1),
        .msg = "tx_ptp_fifo_err_int_sts",
        .type_id = FIFO_ERROR,
        .reset_level = HNAE3_NONE_RESET
    }
};

typedef struct {
    enum hclge_plf_mod_name_list module_id;
    const struct hclge_plf_hw_error *err_int;
    u8 ras_int_num;
} ras_mod_msg;

STATIC ras_mod_msg ras_mod_msg_array[] = {
    { MODULE_IGU_EGU, g_hclge_plf_igu_egu_err_int, ARRAY_SIZE(g_hclge_plf_igu_egu_err_int) },
    { MODULE_IGU_EGU, g_hclge_plf_xxvge_err_int, ARRAY_SIZE(g_hclge_plf_xxvge_err_int) },
    { MODULE_PPP, g_hclge_plf_ppp_fifo_err_int, ARRAY_SIZE(g_hclge_plf_ppp_fifo_err_int) },
    { MODULE_PPP, g_hclge_plf_ppp_mem_sbit_err_int, ARRAY_SIZE(g_hclge_plf_ppp_mem_sbit_err_int) },
    { MODULE_PPP, g_hclge_plf_ppp_mem_mbit_err_int, ARRAY_SIZE(g_hclge_plf_ppp_mem_mbit_err_int) },
    { MODULE_SSU, g_hclge_plf_ssu_err_int, ARRAY_SIZE(g_hclge_plf_ssu_err_int) },
    { MODULE_PPU, g_hclge_plf_tpu_err_int, ARRAY_SIZE(g_hclge_plf_tpu_err_int) },
    { MODULE_PPU, g_hclge_plf_rpu_pf_err_int, ARRAY_SIZE(g_hclge_plf_rpu_pf_err_int) },
    { MODULE_PPU, g_hclge_plf_rpu_mode_err_int, ARRAY_SIZE(g_hclge_plf_rpu_mode_err_int) },
    { MODULE_PPU, g_hclge_plf_rcb_mpf_err_int, ARRAY_SIZE(g_hclge_plf_rcb_mpf_err_int) },
    { MODULE_PPU, g_hclge_plf_rcb_pf_err_int, ARRAY_SIZE(g_hclge_plf_rcb_pf_err_int) }
};

STATIC const struct hclge_plf_hw_type_id g_hclge_plf_hw_type_id_st[] = {
    {
        .type_id = NONE_ERROR,
        .msg = "none_error"
    }, {
        .type_id = FIFO_ERROR,
        .msg = "fifo_error"
    }, {
        .type_id = MEM_MBECC_ERROR,
        .msg = "mbecc_error"
    }, {
        .type_id = ABNORMAL_ERROR,
        .msg = "abnormal_error"
    }, {
        .type_id = BUFFER_ERROR,
        .msg = "buffer_error"
    }, {
        .type_id = MEM_SBECC_ERROR,
        .msg = "sbecc_error"
    }
};

STATIC const struct hclge_plf_module_id g_hclge_plf_module_id_st[] = {
    {
        .module_id = MODULE_IGU_EGU,
        .msg = "MODULE_IGU_EGU"
    }, {
        .module_id = MODULE_PPP,
        .msg = "MODULE_PPP"
    }, {
        .module_id = MODULE_SSU,
        .msg = "MODULE_SSU"
    }, {
        .module_id = MODULE_PPU,
        .msg = "MODULE_PPU"
    }, {
        .module_id = MODULE_NONE,
        .msg = "MODULE_NONE"
    }
};

STATIC int hclge_plf_fault_ops_init(struct dms_node *device);
STATIC void hclge_plf_fault_ops_uninit(struct dms_node *device);
STATIC void hclge_plf_fault_handler(const struct notify_data *pdata);

STATIC struct dfm_struct g_hclge_plf_fault_dms = {0};

STATIC struct dms_node_operations g_hclge_plf_fault_ops = {
    .init = hclge_plf_fault_ops_init,
    .uninit = hclge_plf_fault_ops_uninit,
    .get_info_list = NULL,
    .get_state = NULL,
    .get_capacity = NULL,
    .set_power_state = NULL,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

STATIC struct dms_node g_dms_node_table[HCLGE_PLF_NODE_NUM] = {
    HCLGE_PLF_NODE_DEFINE(DMS_DEV_TYPE_NIC, HCLGE_PLF_NODE_NIC0, &g_hclge_plf_fault_ops),
    HCLGE_PLF_NODE_DEFINE(DMS_DEV_TYPE_NIC, HCLGE_PLF_NODE_NIC1, &g_hclge_plf_fault_ops),
    HCLGE_PLF_NODE_DEFINE(DMS_DEV_TYPE_NIC, HCLGE_PLF_NODE_NIC2, &g_hclge_plf_fault_ops),
    HCLGE_PLF_NODE_DEFINE(DMS_DEV_TYPE_NIC, HCLGE_PLF_NODE_NIC3, &g_hclge_plf_fault_ops),
};

STATIC struct hclge_plf_fault_info g_hclge_plf_fault_tables[] = {
    HCLGE_PLF_RAS_FAULT_EVENT(HISI_SUBSYS_NICSUB, HISI_MODULE_NIC, FIFO_ERROR, "fifo error", RAS_ERROR_TYPE_ERROR),
    HCLGE_PLF_RAS_FAULT_EVENT(HISI_SUBSYS_NICSUB, HISI_MODULE_NIC, MEM_MBECC_ERROR, "mem error", RAS_ERROR_TYPE_MBECC),
    HCLGE_PLF_RAS_FAULT_EVENT(HISI_SUBSYS_NICSUB, HISI_MODULE_NIC, ABNORMAL_ERROR, "abn error", RAS_ERROR_TYPE_ERROR),
    HCLGE_PLF_RAS_FAULT_EVENT(HISI_SUBSYS_NICSUB, HISI_MODULE_NIC, BUFFER_ERROR, "buffer error", RAS_ERROR_TYPE_ERROR),
    HCLGE_PLF_RAS_FAULT_EVENT(HISI_SUBSYS_NICSUB, HISI_MODULE_NIC, MEM_SBECC_ERROR, "sbecc_error",
        RAS_ERROR_TYPE_SBECCOverThold),
};

#ifndef DEFINE_HNS_LLT
STATIC void hclge_plf_notify_sensor_scan(struct dfm_struct *dfm, u32 node_idx, u8 sensor_type)
{
    struct dfm_node *dev_node = &dfm->dev_nodes[node_idx];
    u32 i;

    for (i = 0; i < dev_node->sensor_num; i++) {
        if (dev_node->sensors[i].dms_sensor.sensor_type == sensor_type) {
            /* notify the sensor to scan event immediately */
            if (dms_sensor_event_notify(0, &dev_node->sensors[i].dms_sensor) != 0) {
                pr_err("[hclge_plf]: hclge plf dms sensor event notify error.\n");
            }
        }
    }
}
#endif

STATIC int hclge_plf_fault_scan_event(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 sensor_type;
    u32 node_id;
    int ret;

    if (data == NULL) {
        pr_err("[hclge_plf]: invalid parameter, data is null.\n");
        return -EINVAL;
    }

    node_id = ((private_data >> DEV_NODE_OFFSET) & DEV_NODE_MASK);
    sensor_type = ((private_data >> SENSOR_TYPE_OFFSET) & SENSOR_TYPE_MASK);

    if (node_id >= g_hclge_plf_fault_dms.node_num) {
        pr_err("[hclge_plf]: invalid hclge_plf node id(%u).\n", node_id);
        return -EINVAL;
    }

    ret = dfm_scan_events(&g_hclge_plf_fault_dms, node_id, sensor_type, data);
    if (ret != 0) {
        pr_err("[hclge_plf]: dfm scan hclge_plf fault event failed, ret = %d.\n", ret);
        return -EFAULT;
    }

    return 0;
}

STATIC int hclge_plf_fault_ops_init(struct dms_node *device)
{
    pr_info("[hclge_plf]: hclge plf fault node ops init\n");
    return 0;
}

STATIC void hclge_plf_fault_ops_uninit(struct dms_node *device)
{
    pr_info("[hclge_plf]: hclge plf fault node ops exit\n");
}

STATIC struct dms_sensor_object_cfg g_hclge_plf_fault_sensor_table[HCLGE_PLF_SENSOR_NUM] = {
    { /* RAS故障 */
        .sensor_type = DMS_SEN_TYPE_RAS_SENSOR,
        .sensor_name = "nic_ras",
        .sensor_class = DMS_DISCRETE_SENSOR_CLASS,
        .sensor_class_cfg = {
            .discrete_sensor = {
                .attribute = DMS_SENSOR_ATTRIB_THRES_NONE,
                .debounce_time = 0,
            }
        },
        .scan_interval = DMS_SENSOR_SCAN_INTERVAL,
        .proc_flag = DMS_SENSOR_PROC_ENABLE_FLAG,
        .enable_flag = DMS_SENSOR_ENABLE_FALG,
        .assert_event_mask = 0xFFFF,
        .deassert_event_mask = 0xFFBF,
        .pf_scan_func = hclge_plf_fault_scan_event,
    }
};

#ifndef DEFINE_HNS_LLT
STATIC int hclge_plf_init_dms_node(void)
{
    struct dms_sensor_object_cfg *sensor_config;
    struct dfm_struct *hclge_plf_dms;
    u32 sensor_idx, sensor_num;
    u32 node_idx;

    hclge_plf_dms = &g_hclge_plf_fault_dms;
    if (hclge_plf_dms->dev_nodes == NULL) {
        pr_err("[hclge_plf]: failed to init g_hclge_plf_fault_dms\n");
        return -EINVAL;
    }

    hclge_plf_dms->dev_id = 0;

    /* init dms node and sensor */
    for (node_idx = 0; node_idx < HCLGE_PLF_NODE_NUM; node_idx++) {
        hclge_plf_dms->dev_nodes[node_idx].node = &g_dms_node_table[node_idx];
        hclge_plf_dms->dev_nodes[node_idx].post_proc = NULL;
        hclge_plf_dms->dev_nodes[node_idx].fpdc_notify = hclge_plf_fault_handler;

        sensor_num = hclge_plf_dms->dev_nodes[node_idx].sensor_num;
        for (sensor_idx = 0; sensor_idx < sensor_num; sensor_idx++) {
            sensor_config = &g_hclge_plf_fault_sensor_table[sensor_idx];
            sensor_config->private_data = SENSOR_PRIV_DATA(node_idx, sensor_config->sensor_type);
            hclge_plf_dms->dev_nodes[node_idx].sensors[sensor_idx].dms_sensor = *sensor_config;
        }
    }

    return 0;
}
#endif

int hclge_plf_register_dms_node(void)
{
    int ret;

    ret = dfm_struct_init(&g_hclge_plf_fault_dms, HCLGE_PLF_NODE_NUM, HCLGE_PLF_SENSOR_NUM);
    if (ret != 0) {
        pr_err("[hclge_plf]: failed to init hclge_plf dms struct, ret = %d.\n", ret);
        return -EINVAL;
    }

#ifndef DEFINE_HNS_LLT
    ret = hclge_plf_init_dms_node();
    if (ret != 0) {
        pr_err("[hclge_plf]: failed to init hclge_plf dms node, ret = %d.\n", ret);
        goto dms_node_fail;
    }
#endif

    ret = dfm_register_nodes(&g_hclge_plf_fault_dms);
    if (ret != 0) {
        pr_err("[hclge_plf]: failed to register hclge_plf dms node, ret = %d.\n", ret);
        goto dms_node_fail;
    }

    return 0;

dms_node_fail:
    dfm_struct_final(&g_hclge_plf_fault_dms, HCLGE_PLF_NODE_NUM, HCLGE_PLF_SENSOR_NUM);
    return -EFAULT;
}

void hclge_plf_unregister_dms_node(void)
{
    dfm_unregister_nodes(&g_hclge_plf_fault_dms);
    dfm_struct_final(&g_hclge_plf_fault_dms, HCLGE_PLF_NODE_NUM, HCLGE_PLF_SENSOR_NUM);
}

STATIC void hclge_plf_config_ssu_err_init(struct hclge_plf_dev *hdev)
{
    u32 reg_addr;

    reg_addr = SSU_REG_BASE + PPE_SSU_SSU_INT_TYPE_REG;
    hclge_write_dev(&hdev->hw, reg_addr, 0x20AAAAA9); /* mem 1bit：CE, others: NFE */

    reg_addr = SSU_REG_BASE + PPE_SSU_SSU_INT_ENABLE_REG;
    hclge_write_dev(&hdev->hw, reg_addr, 0x4FFF);
}

STATIC void hclge_plf_config_rcb_mpf_err_init(struct hclge_plf_dev *hdev)
{
    u32 reg_addr;

    reg_addr = RCB_COM_REG_BASE + PPE_RCB_COM_MPF_ABNORMAL_INT_TYPE_REG;
    hclge_write_dev(&hdev->hw, reg_addr, 0xAAA); /* all RAS: NFE */

    reg_addr = RCB_COM_REG_BASE + PPE_RCB_COM_MPF_ABNORMAL_INT_EN_REG;
    hclge_write_dev(&hdev->hw, reg_addr, 0x3F);
}

STATIC void hclge_plf_config_rcb_spf_err_init(struct hclge_plf_dev *hdev)
{
    u8 port_id = hdev->id;
    u32 reg_addr;

    reg_addr = RCB_COM_REG_BASE + PPE_RCB_COM_PF_ABNORMAL_INT_TYPE_0_REG + port_id * 0x80;
    hclge_write_dev(&hdev->hw, reg_addr, 0x2AA); /* all RAS: NFE */

    reg_addr = RCB_COM_REG_BASE + PPE_RCB_COM_PF_ABNORMAL_INT_EN_0_REG + port_id * 0x80;
    hclge_write_dev(&hdev->hw, reg_addr, 0x1F);
}

STATIC void hclge_plf_config_tpu_err_init(struct hclge_plf_dev *hdev)
{
    u32 reg_addr;

    reg_addr = TPU_REG_BASE + PPE_TPU_TPU_PF_INT_TYPE0_REG;
    hclge_write_dev(&hdev->hw, reg_addr, 0xAA9); /* mem 1bit：CE, others: NFE */

    reg_addr = TPU_REG_BASE + PPE_TPU_TPU_INT_ENABLE_REG;
    hclge_write_dev(&hdev->hw, reg_addr, 0x3F);
}

STATIC void hclge_plf_config_rpu_err_init(struct hclge_plf_dev *hdev)
{
    u32 reg_addr;

    reg_addr = RPU_REG_BASE + PPE_RPU_RPU_PF_INT_TYPE0_REG;
    hclge_write_dev(&hdev->hw, reg_addr, 0x2AA9); /* mem 1bit：CE, others: NFE */

    reg_addr = RPU_REG_BASE + PPE_RPU_RPU_PF_INT_ENABLE0_REG;
    hclge_write_dev(&hdev->hw, reg_addr, 0x3F);
}

STATIC void hclge_plf_config_ppp_err_init(struct hclge_plf_dev *hdev)
{
    u32 reg_addr;

    reg_addr = PPP_REG_BASE + PPE_PPP_PPP_INT_TYPE_REG;
    hclge_write_dev(&hdev->hw, reg_addr, 0x2AA9); /* mem 1bit：CE, others: NFE */

    reg_addr = PPP_REG_BASE + PPE_PPP_PPP_PART_INT_ENABLE_REG;
    hclge_write_dev(&hdev->hw, reg_addr, 0x3);
    reg_addr = PPP_REG_BASE + PPE_PPP_PPP_MEM_SBIT_ECC_ENABLE_REG;
    hclge_write_dev(&hdev->hw, reg_addr, 0x1000);
    reg_addr = PPP_REG_BASE + PPE_PPP_PPP_MEM_MBIT_ECC_ENABLE_REG;
    hclge_write_dev(&hdev->hw, reg_addr, 0x1000);
}

STATIC void hclge_plf_config_igu_egu_err_init(struct hclge_plf_dev *hdev)
{
    u8 mac_id = hdev->hw.mac.mac_id;
    u32 reg_addr;

    reg_addr = IGU_EGU_CFG_IGU_EGU_TNL_INT_RPT_TYPE_0_REG + mac_id * 0x100;
    hclge_write_dev(&hdev->hw, reg_addr, 0x2AA); /* all RAS: NFE */

    reg_addr = IGU_EGU_CFG_IGU_EGU_TNL_INT_ENABLE_0_REG + mac_id * 0x100;
    hclge_write_dev(&hdev->hw, reg_addr, 0x1F);
}

STATIC void hclge_plf_config_xxvge_err_init(struct hclge_plf_dev *hdev)
{
    u32 reg_addr;

    reg_addr = XXVGE_COMMON_AFIFO_TNL_INT_RPT_TYPE_REG;
    hclge_write_dev(&hdev->hw, reg_addr, 0xA); /* all RAS: NFE */

    reg_addr = XXVGE_COMMON_AFIFO_TNL_INT_ENABLE_REG;
    hclge_write_dev(&hdev->hw, reg_addr, 0x3);
}

void hclge_plf_ras_int_config(struct hclge_plf_dev *hdev)
{
    hclge_plf_config_ssu_err_init(hdev);
    hclge_plf_config_rcb_mpf_err_init(hdev);
    hclge_plf_config_tpu_err_init(hdev);
    hclge_plf_config_rpu_err_init(hdev);
    hclge_plf_config_ppp_err_init(hdev);
    hclge_plf_config_xxvge_err_init(hdev);
    hclge_plf_config_rcb_spf_err_init(hdev);
    hclge_plf_config_igu_egu_err_init(hdev);
}

STATIC void hclge_plf_error_handle(struct device *dev, const ras_mod_msg *err, u8 *type_id,
    u32 err_sts, unsigned long *reset_requests)
{
    const struct hclge_plf_hw_error *err_int = err->err_int;
    u8 err_int_num = err->ras_int_num;

    while (err_int_num > 0 && err_int->msg) {
        if (err_int->int_msk & err_sts) {
            *type_id = err_int->type_id;
            dev_info(dev, "%s found [error status = 0x%x]\n", err_int->msg, err_sts);
            if (err_int->reset_level && err_int->reset_level != HNAE3_NONE_RESET) {
                set_bit(err_int->reset_level, reset_requests);
            }
        }
        err_int++;
        err_int_num--;
    }
}

STATIC void hclge_plf_handle_err_reset_request(struct hclge_plf_dev *hdev)
{
    struct hnae3_ae_dev *ae_dev = platform_get_drvdata(hdev->pdev);
    struct hnae3_handle *handle = &hdev->vport[0].nic;

    if (ae_dev->hw_err_reset_req) {
        enum hnae3_reset_type reset_type;

        reset_type = hclge_plf_get_reset_level(ae_dev, &(ae_dev->hw_err_reset_req));
        hclge_plf_set_def_reset_request(ae_dev, reset_type);
    }

    if (hdev->default_reset_request && ae_dev->ops->reset_event) {
        ae_dev->ops->reset_event(NULL, handle);
    }
}

STATIC void hclge_plf_fault_report_excep(u32 nic_id, u32 excep_id)
{
    struct hclge_plf_fault_info fault_info;
    u16 size, i;
    int ret;

    if (nic_id >= HClGE_PLF_NODE_ID_MAX) {
        pr_err("[hclge_plf]: %s nic_id[%u] invalid, valid range [0, %u).\n",
            __func__, nic_id, HClGE_PLF_NODE_ID_MAX);
        return;
    }

    size = sizeof(g_hclge_plf_fault_tables) / sizeof(g_hclge_plf_fault_tables[0]);
    for (i = 0; i < size; i++) {
        fault_info = g_hclge_plf_fault_tables[i];
        if (excep_id == fault_info.excep_id) {
            ret = dfm_add_event(&g_hclge_plf_fault_dms, nic_id, &fault_info.event);
            if ((ret != 0) && (ret != -EEXIST)) {
                pr_err("[hclge_plf]: dfm_add_event failed, ret = %d.\n", ret);
                return;
            }
#ifndef DEFINE_HNS_LLT
            hclge_plf_notify_sensor_scan(&g_hclge_plf_fault_dms, nic_id, fault_info.event.sensor_type);
#endif
            return;
        }
    }

    pr_err("[hclge_plf]: report excep failed, excep_id[0x%x] not found\n", excep_id);
    return;
}

STATIC void hclge_plf_fault_report_recovery_excep(u32 nic_id, u32 excep_id)
{
    struct hclge_plf_fault_info fault_info;
    u16 size, i;
    int ret;

    size = sizeof(g_hclge_plf_fault_tables) / sizeof(g_hclge_plf_fault_tables[0]);
    for (i = 0; i < size; i++) {
        fault_info = g_hclge_plf_fault_tables[i];
        if (excep_id == fault_info.excep_id) {
            ret = dfm_remove_event(&g_hclge_plf_fault_dms, nic_id, &fault_info.event);
            if ((ret != 0) && (ret != -EEXIST)) {
                pr_err("[hclge_plf]: dfm_remove_event failed, ret = %d.\n", ret);
            }

            return;
        }
    }

    pr_err("[hclge_plf]: report recovery excep failed, excep_id[0x%x] not found\n", excep_id);
    return;
}

STATIC void hclge_plf_error_log(struct device *dev, struct hclge_plf_ras_error *err)
{
#define RAS_REPORT_INTERVAL 100
    struct hnae3_ae_dev *ae_dev = NULL;
    u32 index_module = MODULE_NONE;
    u32 total_module, total_type;
    u32 index_type = NONE_ERROR;
    struct hclge_plf_dev *hdev;
    u32 i;

    ae_dev = dev_get_drvdata(dev);
    hdev = ae_dev->priv;

    total_module = ARRAY_SIZE(g_hclge_plf_module_id_st);
    total_type = ARRAY_SIZE(g_hclge_plf_hw_type_id_st);

    for (i = 0; i < total_module; i++) {
        if (err->sub_module_id == g_hclge_plf_module_id_st[i].module_id) {
            index_module = i;
            break;
        }
    }

    for (i = 0; i < total_type; i++) {
        if (err->type_id == g_hclge_plf_hw_type_id_st[i].type_id) {
            index_type = i;
            break;
        }
    }

    if (index_module != MODULE_NONE && index_type != NONE_ERROR) {
        dev_err(dev, "found %s %s, is ras error.\n",
                g_hclge_plf_module_id_st[index_module].msg,
                g_hclge_plf_hw_type_id_st[index_type].msg);
        dev_err(dev, "reg_value:\n");
        for (i = 0; i < err->register_array_size; i++) {
            dev_err(dev, "0x%08x\n", err->register_array[i]);
        }
#ifndef DEFINE_HNS_LLT
        hclge_plf_fault_report_excep(err->sensor_id, HCLGE_PLF_EVENT_EXCEP_ID(index_type, err->type_id));
        hclge_plf_handle_err_reset_request(hdev);
        mdelay(RAS_REPORT_INTERVAL);
        if (index_type != MEM_SBECC_ERROR) {
            hclge_plf_fault_report_recovery_excep(err->sensor_id, HCLGE_PLF_EVENT_EXCEP_ID(index_type, err->type_id));
        }

#endif
    } else {
        dev_err(dev, "unknown module[%d] or type[%d].\n", err->sub_module_id, err->type_id);
    }
}

STATIC void hclge_plf_hw_fault_handler(struct hclge_plf_dev *hdev, const hisi_common_error_info *err_data)
{
    struct hclge_plf_ras_error error_info = {0};
    struct hnae3_ae_dev *ae_dev = hdev->ae_dev;
    struct device *dev = &hdev->pdev->dev;
    u32 register_array_size, arr_size;
    u8 sub_module_id, type_id;
    const u32 *register_array;
    u32 err_sts;
    u32 i, j;

    register_array_size = err_data->register_array_size;
    register_array = err_data->register_array;
    sub_module_id = err_data->sub_module_id;

    arr_size = (u32)(sizeof(ras_mod_msg_array) / sizeof(ras_mod_msg_array[0]));
    for (i = 0, j = 0; (i < arr_size) && (j < register_array_size); i++) {
        if (sub_module_id == ras_mod_msg_array[i].module_id) {
            err_sts = register_array[j];
            hclge_plf_error_handle(dev, (ras_mod_msg_array + i), &type_id, err_sts, &ae_dev->hw_err_reset_req);
            j++;
        }
    }

    error_info.register_array = register_array;
    error_info.register_array_size = register_array_size;
    error_info.sub_module_id = sub_module_id;
    error_info.type_id = type_id;
    error_info.sensor_id = err_data->function_or_port_id;

    hclge_plf_error_log(dev, &error_info);
}

STATIC int hclge_plf_check_notify_data(const struct notify_data *pdata)
{
    if (pdata == NULL || pdata->origin_data == NULL) {
        pr_err("[hclge_plf]: fpdc pdata or pdata->origin_data is NULL.\n");
        return -EINVAL;
    }

    if ((pdata->src_type != FPDC_SRC_RAS) || ((pdata->node_type != DMS_DEV_TYPE_NIC) &&
        (pdata->node_type != DMS_DEV_TYPE_PORT))) {
        pr_err("[hclge_plf]: invalid paramters: src_type(%d), node_type(%d)\n", pdata->src_type, pdata->node_type);
        return -EINVAL;
    }

    if (pdata->data_len < sizeof(hisi_common_error_info)) {
        pr_err("[hclge_plf]: invalid data length: len(%u)\n", pdata->data_len);
        return -EINVAL;
    }

    return 0;
}

STATIC void hclge_plf_fault_handler(const struct notify_data *pdata)
{
    const hisi_common_error_info *err_data;
    struct hnae3_ae_dev *ae_dev_next = NULL;
    struct list_head *ae_dev_list = NULL;
    struct hnae3_ae_dev *ae_dev = NULL;
    struct hclge_plf_dev *hdev = NULL;
    u8 port_id;
    int ret;

    ret = hclge_plf_check_notify_data(pdata);
    if (ret != 0) {
        pr_err("[hclge_plf]: fault notify data invalid, ret = %d.\n", ret);
        return;
    }

    err_data = (hisi_common_error_info *)pdata->origin_data;
    port_id = err_data->function_or_port_id;

    hdev = hclge_get_hclge_plf_dev(port_id);
    if (hdev == NULL) {
        pr_err("[hclge_plf]: hclge_plf fault get device failed.\n");
        return;
    }

    if (!test_bit(HCLGE_STATE_SERVICE_INITED, &hdev->state)) {
        dev_err(&hdev->pdev->dev, "can't handle hw RAS error reported during dev init.\n");
        return;
    }

    hclge_plf_hw_fault_handler(hdev, err_data);
}
