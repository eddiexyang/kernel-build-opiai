/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Create: 2023-3-2
 */
#include <linux/printk.h>
#include <linux/slab.h>

#include "dms_define.h"
#include "drv_ras_common.h"
#include "dms_interface.h"
#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "dms_sensor.h"
#include "dfm_dev_register.h"
#include "dfm_report.h"
#include "hns3_bbox.h"
#include "hclge_err.h"
#include "hclge_fault_manager.h"

static int hclge_fault_ops_init(struct dms_node *device)
{
    return 0;
}

static void hclge_fault_ops_uninit(struct dms_node *device)
{
    return;
}

static struct dms_node_operations g_hclge_fault_ops = {
    .init = hclge_fault_ops_init,
    .uninit = hclge_fault_ops_uninit,
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

static struct dfm_struct g_hclge_falut_mgr = {0};
static struct dms_node g_hclge_falut_dms_node[HCLGE_NODE_NUM] = {
    {
        .node_type = DMS_DEV_TYPE_ROCE ,
        .node_id = HCLGE_NODE_ROCE,
        .node_name = "ROCE",
        .ops = &g_hclge_fault_ops
    },
    {
        .node_type = DMS_DEV_TYPE_NIC,
        .node_id = HCLGE_NODE_NIC,
        .node_name = "NIC",
        .ops = &g_hclge_fault_ops
    },
    {
        .node_type = DMS_DEV_TYPE_NIC_DISP,
        .node_id = HCLGE_NODE_NIC_DISP,
        .node_name = "NIC_DISP",
        .ops = &g_hclge_fault_ops
    },
    {
        .node_type = DMS_DEV_TYPE_PORT,
        .node_id = HCLGE_NODE_NIC_PORT,
        .node_name = "NIC_PORT",
        .ops = &g_hclge_fault_ops
    },
};

static int hclge_fault_scan_event(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 sensor_type;
    u32 node_id;
    int ret;

    node_id = ((private_data >> DEV_NODE_OFFSET) & DEV_NODE_MASK);
    sensor_type = ((private_data >> SENSOR_TYPE_OFFSET) & SENSOR_TYPE_MASK);

    if (node_id >= g_hclge_falut_mgr.node_num) {
        pr_err("hns3: invalid dms node id(%u), exceeds %u.\n", node_id, g_hclge_falut_mgr.node_num);
        return -EINVAL;
    }

    ret = dfm_scan_events(&g_hclge_falut_mgr, node_id, sensor_type, data);
    if (ret != 0) {
        pr_err("hns3: dfm scan fault event failed, ret=%d.\n", ret);
        return -EFAULT;
    }

    return 0;
}

static struct dms_sensor_object_cfg g_hclge_fault_sensor_table[HCLGE_NODE_NUM][HCLGE_SENSOR_NUM] = {
    { /* dev_node RoCE */
        { /* sensor RAS */
            .sensor_type = DMS_SEN_TYPE_RAS_SENSOR,
            .sensor_name = "roce_ras",
            .sensor_class = DMS_DISCRETE_SENSOR_CLASS,
            .sensor_class_cfg = {
                .discrete_sensor = {
                    .attribute = DMS_SENSOR_ATTRIB_THRES_NONE,
                    .debounce_time = 0,
                }
            },
            .scan_interval = HCLGE_DMS_SENSOR_SCAN_INTERVAL,
            .proc_flag = DMS_SENSOR_PROC_ENABLE_FLAG,
            .enable_flag = DMS_SENSOR_ENABLE_FALG,
            .assert_event_mask = HCLGE_ASSERT_EVENT_MASK,
            .deassert_event_mask = HCLGE_DEASSERT_SOC_EVENT_MASK,
            .pf_scan_func = hclge_fault_scan_event,
        }
    }, { /* dev_node NIC */
        { /* sensor ras */
            .sensor_type = DMS_SEN_TYPE_RAS_SENSOR,
            .sensor_name = "nic_ras",
            .sensor_class = DMS_DISCRETE_SENSOR_CLASS,
            .sensor_class_cfg = {
                .discrete_sensor = {
                    .attribute = DMS_SENSOR_ATTRIB_THRES_NONE,
                    .debounce_time = 0,
                }
            },
            .scan_interval = HCLGE_DMS_SENSOR_SCAN_INTERVAL,
            .proc_flag = DMS_SENSOR_PROC_ENABLE_FLAG,
            .enable_flag = DMS_SENSOR_ENABLE_FALG,
            .assert_event_mask = HCLGE_ASSERT_EVENT_MASK,
            .deassert_event_mask = HCLGE_DEASSERT_SOC_EVENT_MASK,
            .pf_scan_func = hclge_fault_scan_event,
        }
    }, { /* dev_node dispatch */
        { /* sensor ras */
            .sensor_type = DMS_SEN_TYPE_RAS_SENSOR,
            .sensor_name = "nic_dispatch",
            .sensor_class = DMS_DISCRETE_SENSOR_CLASS,
            .sensor_class_cfg = {
                .discrete_sensor = {
                    .attribute = DMS_SENSOR_ATTRIB_THRES_NONE,
                    .debounce_time = 0,
                }
            },
            .scan_interval = HCLGE_DMS_SENSOR_SCAN_INTERVAL,
            .proc_flag = DMS_SENSOR_PROC_ENABLE_FLAG,
            .enable_flag = DMS_SENSOR_ENABLE_FALG,
            .assert_event_mask = HCLGE_ASSERT_EVENT_MASK,
            .deassert_event_mask = HCLGE_DEASSERT_SOC_EVENT_MASK,
            .pf_scan_func = hclge_fault_scan_event,
        }
    }, { /* dev_node port: */
        { /* sensor safety */
            .sensor_type = DMS_SEN_TYPE_SAFETY_SENSOR,
            .sensor_name = "nic_port_safety",
            .sensor_class = DMS_DISCRETE_SENSOR_CLASS,
            .sensor_class_cfg = {
                .discrete_sensor = {
                    .attribute = DMS_SENSOR_ATTRIB_THRES_NONE,
                    .debounce_time = 0,
                }
            },
            .scan_interval = HCLGE_DMS_SENSOR_SCAN_INTERVAL,
            .proc_flag = DMS_SENSOR_PROC_ENABLE_FLAG,
            .enable_flag = DMS_SENSOR_ENABLE_FALG,
            .assert_event_mask = HCLGE_ASSERT_EVENT_MASK,
            .deassert_event_mask = HCLGE_DEASSERT_SOC_SAFETY_MASK,
            .pf_scan_func = hclge_fault_scan_event,
        }
    }
};

static int hclge_check_notify_data(const struct notify_data *pdata)
{
    if (pdata == NULL || pdata->origin_data == NULL) {
        pr_err("hns3: fpdc pdata or pdata->origin_data is NULL.\n");
        return -EINVAL;
    }

    if ((pdata->src_type != FPDC_SRC_RAS)) {
        pr_err("hns3: invalid paramters: src_type(%d)\n", pdata->src_type);
        return -EINVAL;
    }

    if (pdata->data_len < sizeof(hisi_common_error_info)) {
        pr_err("hns3: invalid data length: len(%d)\n", pdata->data_len);
        return -EINVAL;
    }

    return 0;
}

static const comm_ras_msg g_nic_disp_arr[] = {
    { DISP_ERR_STATUSL, DISP_DATA_RAM_MBECC_VALUE, DISP_DATA_RAM_MBECC, RAS_ERROR_TYPE_MBECC },
    { DISP_ERR_STATUSL, DISP_CMD_RAM_MBECC_VALUE, DISP_COMMAND_RAM_MBECC, RAS_ERROR_TYPE_MBECC },
    { DISP_ERR_STATUSL, DISP_NCDIR_MBECC_VALUE, DISP_NCDIR_MBECC, RAS_ERROR_TYPE_MBECC },
    { DISP_ERR_STATUSL, DISP_DATA_RAM_SBECC_VALUE, DISP_DATA_RAM_SBECC, RAS_ERROR_TYPE_SBECCOverThold },
    { DISP_ERR_STATUSL, DISP_CMD_RAM_SBECC_VALUE, DISP_COMMAND_RAM_SBECC, RAS_ERROR_TYPE_SBECCOverThold },
    { DISP_ERR_STATUSL, DISP_NCDIR_SBECC_VALUE, DISP_NCDIR_SBECC, RAS_ERROR_TYPE_SBECCOverThold },
    { DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_0_VALUE, DISP_ERR_RSP_PORT, RAS_ERROR_TYPE_INPUT_ERR },
    { DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_1_VALUE, DISP_ERR_RSP_PORT, RAS_ERROR_TYPE_INPUT_ERR },
    { DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_2_VALUE, DISP_ERR_RSP_PORT, RAS_ERROR_TYPE_INPUT_ERR },
    { DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_3_VALUE, DISP_ERR_RSP_PORT, RAS_ERROR_TYPE_INPUT_ERR },
    { DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_4_VALUE, DISP_ERR_RSP_PORT, RAS_ERROR_TYPE_INPUT_ERR },
    { DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_5_VALUE, DISP_ERR_RSP_PORT, RAS_ERROR_TYPE_INPUT_ERR },
    { DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_6_VALUE, DISP_ERR_RSP_PORT, RAS_ERROR_TYPE_INPUT_ERR },
    { DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_7_VALUE, DISP_ERR_RSP_PORT, RAS_ERROR_TYPE_INPUT_ERR },
    { DISP_ERR_STATUSL, DISP_ERR_REQ_VALUE, DISP_ERR_REQ, RAS_ERROR_TYPE_CFG_ERR },
    { DISP_ERR_STATUSL, DISP_ERR_WD_VALUE, DISP_ERR_WD, RAS_ERROR_TYPE_INPUT_ERR }
};

static const comm_ras_msg g_nic_bios_comm_arr[] = {
    { LOC_IMP_TCM_ECC_INT_SRC, TCM_MERR_MASK, MEMORY_ERROR, RAS_ERROR_TYPE_MBECC },
    { LOC_CMDQ_MEM_ECC_INT_SRC, CMDQ_MEM_MERR_MASK, MEMORY_ERROR, RAS_ERROR_TYPE_MBECC },
    { LOC_IMP_RD_POISON_INT_SRC, RD_POISON_MASK, POISON_ERROR, RAS_ERROR_TYPE_MBECC },
    { LOC_TQP_INT_ECC_INT_SRC_0, TQP_INT_STS_0_MERR_MASK, TQP_INT_ECC_ERROR, RAS_ERROR_TYPE_MBECC },
    { LOC_MSIX_ECC_INT_SRC, MSIX_MERR_MASK, MSIX_ECC_ERROR, RAS_ERROR_TYPE_MBECC },
    { LOC_TQP_INT_ECC_INT_SRC_1_0, TQP_INT_STS_1_MERR_MASK, TQP_INT_ECC_ERROR, RAS_ERROR_TYPE_MBECC },
    { LOC_TQP_INT_ECC_INT_SRC_1_1, TQP_INT_STS_1_MERR_MASK, TQP_INT_ECC_ERROR, RAS_ERROR_TYPE_MBECC },
    { LOC_TQP_INT_ECC_INT_SRC_1_2, TQP_INT_STS_1_MERR_MASK, TQP_INT_ECC_ERROR, RAS_ERROR_TYPE_MBECC }
};

static int hclge_get_nic_comm_ras(struct hclge_fpdc_ras_error *error_info, const comm_ras_msg *event_list, u8 event_num)
{
#define DISP_VALUE_OFF 8
    u32 reg_array_size, offset;
    const u32 *reg_array;
    u8 i;

    reg_array = error_info->register_array;
    reg_array_size = error_info->register_array_size;

    for (i = 0; i < event_num; i++) {
        offset = event_list[i].ras_reg_offset;
        if (offset >= reg_array_size) {
            pr_err("hns3: invalid event list reg offset(%u), size = %u\n", offset, reg_array_size);
            continue;
        }

        if (((reg_array[offset] & IERR_MASK) >> DISP_VALUE_OFF) == event_list[i].ras_reg_value) {
            error_info->ras_code = event_list[i].ras_code;
            error_info->ras_type = event_list[i].ras_type;
            return 0;
        }
    }

    pr_err("hns3: No match event, (dev_id = %d, module_id = %d)\n", error_info->device_id, error_info->module_id);
    return -EINVAL;
}

static int hclge_get_bios_comm_ras(struct hclge_fpdc_ras_error *error_info, const comm_ras_msg *event_list, u8 event_num)
{
#define DISP_VALUE_OFF 8
    u32 reg_array_size, offset;
    const u32 *reg_array;
    u8 i;

    reg_array = error_info->register_array;
    reg_array_size = error_info->register_array_size;

    for (i = 0; i < event_num; i++) {
        offset = event_list[i].ras_reg_offset;
        if (offset >= reg_array_size) {
            pr_err("hns3: invalid event list reg offset(%u), size = %u\n", offset, reg_array_size);
            continue;
        }

        if ((reg_array[offset] & event_list[i].ras_reg_mask) != 0 ) {
            error_info->ras_code = event_list[i].ras_code;
            error_info->ras_type = event_list[i].ras_type;
            return 0;
        }
    }

    pr_err("hns3: No match event, (dev_id = %d, module_id = %d)\n", error_info->device_id, error_info->module_id);
    return -EINVAL;
}

static int hclge_get_nic_aa_comm_ras(struct hclge_fpdc_ras_error *error_info)
{
    pr_warn("hns3: module NIC_AA can not report RAS event!\n");
    return -EINVAL;
}

static int hclge_get_nic_disp_comm_ras(struct hclge_fpdc_ras_error *error_info)
{
    const comm_ras_msg *disp_event_list = g_nic_disp_arr;
    u8 event_num = sizeof(g_nic_disp_arr) / sizeof(comm_ras_msg);

    return hclge_get_nic_comm_ras(error_info, disp_event_list, event_num);
}

static int hclge_get_nic_smmu_comm_ras(struct hclge_fpdc_ras_error *error_info)
{
    pr_warn("hns3: module NIC_SMMU can not report RAS event!\n");
    return -EINVAL;
}

static int hclge_get_nic_sch_comm_ras(struct hclge_fpdc_ras_error *error_info)
{
    pr_warn("hns3: module NIC_SCH can not report RAS event!\n");
    return -EINVAL;
}

static int hclge_get_nic_bios_comm_ras(struct hclge_fpdc_ras_error *error_info)
{
    const comm_ras_msg *bios_common_event_list = g_nic_bios_comm_arr;
    u8 event_num = sizeof(g_nic_bios_comm_arr) / sizeof(comm_ras_msg);

    return hclge_get_bios_comm_ras(error_info, bios_common_event_list, event_num);
}

static int hclge_get_nic_comm_ras_info(struct hclge_fpdc_ras_error *error_info)
{
#define BIOS_COMMON_SUB_MODULE_ID 5
    u8 module_id = error_info->module_id;
    u8 sub_module_id = error_info->sub_module_id;

    switch (module_id) {
        case HISI_MODULE_AA:
             return hclge_get_nic_aa_comm_ras(error_info);
        case HISI_MODULE_DISP:
            return hclge_get_nic_disp_comm_ras(error_info);
        case HISI_MODULE_SMMU:
            return hclge_get_nic_smmu_comm_ras(error_info);
        case HISI_MODULE_SCH:
            return hclge_get_nic_sch_comm_ras(error_info);
        case HISI_MODULE_NIC:
            if (sub_module_id == BIOS_COMMON_SUB_MODULE_ID) {
                return hclge_get_nic_bios_comm_ras(error_info);
            }
        default:
            break;
    }

    pr_warn("hns3: this module does not use FPDC to report RAS, module_id(%d), sub_module_id(%d), device_id(%d)",
            module_id, sub_module_id, error_info->device_id);
    return -EINVAL;
}

static void hclge_hw_fault_handler(const struct notify_data *pdata)
{
#define RAS_ERROR_TYPE_OFFSET 8
    struct hclge_fpdc_ras_error error_info = {0};
    const hisi_common_error_info *err_data;
    int ret;

    err_data = (hisi_common_error_info *)pdata->origin_data;

    error_info.device_id = pdata->chip_id;
    error_info.module_id = err_data->module_id;
    error_info.sub_module_id = err_data->sub_module_id;
    error_info.register_array_size = err_data->register_array_size;
    error_info.register_array = err_data->register_array;

    ret = hclge_get_nic_comm_ras_info(&error_info);
    if (ret != 0)  {
        pr_err("hns3: failed to get nic comm ras, ret = %d\n", ret);
        return;
    }

    hclge_fault_report_excep(error_info.device_id,
                             ((error_info.ras_code << RAS_ERROR_TYPE_OFFSET) | (u32)error_info.ras_type));
    /* The NIC fault can be rectified by reset. Therefore, the recovery event needs to be reported. */
    if (error_info.module_id == HISI_MODULE_NIC) {
        msleep(HCLGE_DMS_SENSOR_SCAN_INTERVAL);
        hclge_fault_report_recovery_excep(error_info.device_id,
                                          ((error_info.ras_code << RAS_ERROR_TYPE_OFFSET) | (u32)error_info.ras_type));
    }
}

static void hclge_common_fault_handler(const struct notify_data *pdata)
{
    int ret;

    ret = hclge_check_notify_data(pdata);
    if (ret != 0) {
        pr_err("hns3: fault notify data invalid, ret = %d.\n", ret);
        return;
    }

    hclge_hw_fault_handler(pdata);
}

static void hclge_fault_dev_nodes_init(void)
{
    struct dms_sensor_object_cfg *sensor_config;
    int i, j;

    for (i = 0; i < HCLGE_NODE_NUM; i++) {
        g_hclge_falut_dms_node[i].owner_devid = (int)(g_hclge_falut_mgr.dev_id);
        g_hclge_falut_mgr.dev_nodes[i].node = &g_hclge_falut_dms_node[i];
        g_hclge_falut_mgr.dev_nodes[i].post_proc = NULL;

        /* Compatible with current fpdc reporting framework */
        if (g_hclge_falut_dms_node[i].node_type == DMS_DEV_TYPE_NIC_DISP || DMS_DEV_TYPE_NIC) {
            g_hclge_falut_mgr.dev_nodes[i].fpdc_notify = hclge_common_fault_handler;
        } else {
            g_hclge_falut_mgr.dev_nodes[i].fpdc_notify = NULL;
        }

        for (j = 0; j < HCLGE_SENSOR_NUM; j++) {
            sensor_config = &g_hclge_fault_sensor_table[i][j];
            sensor_config->private_data = SENSOR_PRIV_DATA(g_hclge_falut_mgr.dev_id, i,
                                                           sensor_config->sensor_type);
            g_hclge_falut_mgr.dev_nodes[i].sensors[j].dms_sensor = *sensor_config;
        }
    }
}

int hclge_fault_mgr_init(struct hclge_dev *hdev)
{
    int ret;

    g_hclge_falut_mgr.dev_id = hdev->pf_id;

    ret = dfm_struct_init(&g_hclge_falut_mgr, HCLGE_NODE_NUM, HCLGE_SENSOR_NUM);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "init dfm data failed, ret = %d\n", ret);
        goto dfm_fail;
    }

    hclge_fault_dev_nodes_init();

    ret = dfm_register_nodes(&g_hclge_falut_mgr);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "register dms node failed, ret = %d\n", ret);
        goto dfm_fail;
    }

    return 0;

dfm_fail:
    dfm_struct_final(&g_hclge_falut_mgr, HCLGE_NODE_NUM, HCLGE_SENSOR_NUM);
    return ret;
}

void hclge_fault_mgr_unint(void)
{
    dfm_unregister_nodes(&g_hclge_falut_mgr);
    dfm_struct_final(&g_hclge_falut_mgr, HCLGE_NODE_NUM, HCLGE_SENSOR_NUM);
}

/* Due to not using fpdc to obtain RAS specific information, sub_id/mod_id is non-standard usage */
static struct hclge_fault_info g_hclge_fault_infos[] = {
    { HNS_EXCEPID_IMP_HARD_FAULT, HCLGE_BBOX_FAULT_EVENT(NIC_IMP_HARD_FAULT, "imp hard fault") },
    { HNS_EXCEPID_IMP_MEM_FAULT, HCLGE_BBOX_FAULT_EVENT(NIC_IMP_MEM_FAULT, "imp mem fault") },
    { HNS_EXCEPID_IMP_BUS_FAULT, HCLGE_BBOX_FAULT_EVENT(NIC_IMP_BUS_FAULT, "imp bus fault") },
    { HNS_EXCEPID_IMP_USAGE_FAULT, HCLGE_BBOX_FAULT_EVENT(NIC_IMP_USAGE_FAULT, "imp usage fault") },
    { HNS_EXCEPID_IMP_SVCALL_FAULT, HCLGE_BBOX_FAULT_EVENT(NIC_IMP_SVCALL_FAULT, "imp svcall fault") },
    { HNS_EXCEPID_IMP_DEBUG_HANDLE, HCLGE_BBOX_FAULT_EVENT(NIC_IMP_DEBUG_HANDLE, "imp debug handle") },
    { HNS_EXCEPID_IMP_PENDSV, HCLGE_BBOX_FAULT_EVENT(NIC_IMP_PENDSV, "imp pendsv") },
    { HNS_EXCEPID_IMP_UNHANDLE_FAULT, HCLGE_BBOX_FAULT_EVENT(NIC_IMP_UNHANDLE_FAULT, "imp unhandle fault") },
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, FIFO_ERROR, "fifo error", RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, FIFO_ERROR, "fifo error", RAS_ERROR_TYPE_MBECC),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, MEMORY_ERROR, "memory error", RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, MEMORY_ERROR, "memory error", RAS_ERROR_TYPE_MBECC),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, POISON_ERROR, "poison error", RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, POISON_ERROR, "poison error", RAS_ERROR_TYPE_MBECC),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, MSIX_ECC_ERROR, "msix ecc error", RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, MSIX_ECC_ERROR, "msix ecc error", RAS_ERROR_TYPE_MBECC),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, TQP_INT_ECC_ERROR, "tqp int ecc error",
        RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, TQP_INT_ECC_ERROR, "tqp int ecc error",
        RAS_ERROR_TYPE_MBECC),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, PF_ABNORMAL_INT_ERROR, "pf abnormal int error",
        RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, PF_ABNORMAL_INT_ERROR, "pf abnormal int error",
        RAS_ERROR_TYPE_MBECC),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, MPF_ABNORMAL_INT_ERROR, "mpf abnormal int error",
        RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, MPF_ABNORMAL_INT_ERROR, "mpf abnormal int error",
        RAS_ERROR_TYPE_MBECC),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, COMMON_ERROR, "common error", RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, COMMON_ERROR, "common error", RAS_ERROR_TYPE_MBECC),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, PORT_ERROR, "port error", RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, ETS_ERROR, "ets error", RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, GLB_ERROR, "glb error", RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, LINK_ERROR, "link error", RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, PTP_ERROR, "ptp error", RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_ROCE, ROCEE_NORMAL_ERR, "rocee normal error",
        RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_ROCE, ROCEE_NORMAL_ERR, "rocee normal error",
        RAS_ERROR_TYPE_MBECC),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_ROCE, ROCEE_OVF_ERR, "rocee ovf error", RAS_ERROR_TYPE_ERROR),
    HCLGE_RAS_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_ROCE, ROCEE_BUS_ERR, "rocee bus error", RAS_ERROR_TYPE_BUS_ERR),
    HCLGE_DISP_FAULT_EVENT(HISI_SUBSYS_NICSUB, HISI_MODULE_DISP, DISP_DATA_RAM_MBECC, "disp data ram mbecc",
        RAS_ERROR_TYPE_MBECC),
    HCLGE_DISP_FAULT_EVENT(HISI_SUBSYS_NICSUB, HISI_MODULE_DISP, DISP_COMMAND_RAM_MBECC, "disp command ram mbecc",
        RAS_ERROR_TYPE_MBECC),
    HCLGE_DISP_FAULT_EVENT(HISI_SUBSYS_NICSUB, HISI_MODULE_DISP, DISP_NCDIR_MBECC, "disp ncdir mbecc",
        RAS_ERROR_TYPE_MBECC),
    HCLGE_DISP_FAULT_EVENT(HISI_SUBSYS_NICSUB, HISI_MODULE_DISP, DISP_DATA_RAM_SBECC, "disp data ram sbecc",
        RAS_ERROR_TYPE_SBECCOverThold),
    HCLGE_DISP_FAULT_EVENT(HISI_SUBSYS_NICSUB, HISI_MODULE_DISP, DISP_COMMAND_RAM_SBECC, "disp command ram sbecc",
        RAS_ERROR_TYPE_SBECCOverThold),
    HCLGE_DISP_FAULT_EVENT(HISI_SUBSYS_NICSUB, HISI_MODULE_DISP, DISP_NCDIR_SBECC, "disp ncdir sbecc",
        RAS_ERROR_TYPE_SBECCOverThold),
    HCLGE_DISP_FAULT_EVENT(HISI_SUBSYS_NICSUB, HISI_MODULE_DISP, DISP_ERR_RSP_PORT, "disp rsp port err",
        RAS_ERROR_TYPE_INPUT_ERR),
    HCLGE_DISP_FAULT_EVENT(HISI_SUBSYS_NICSUB, HISI_MODULE_DISP, DISP_ERR_REQ, "disp req err",
        RAS_ERROR_TYPE_CFG_ERR),
    HCLGE_DISP_FAULT_EVENT(HISI_SUBSYS_NICSUB, HISI_MODULE_DISP, DISP_ERR_WD, "disp wd err",
        RAS_ERROR_TYPE_INPUT_ERR),
    HCLGE_SAFETY_SENSOR_FAULT_EVENT(HISI_SUBSYS_MAX, HISI_MODULE_NIC, LINK_ERROR, "port link status change",
        SOC_SAFETY_LINK_STATE_CHG)
};

static int hclge_notify_sensor_scan(u32 node_id, u8 sensor_type)
{
    struct dfm_node *dev_node;
    int ret;
    u32 i;

    if (node_id >= g_hclge_falut_mgr.node_num) {
        pr_err("hns3: node_id[%u] exceeds %u\n", node_id, g_hclge_falut_mgr.node_num);
        return -EINVAL;
    }

    dev_node = &g_hclge_falut_mgr.dev_nodes[node_id];
    for (i = 0; i < dev_node->sensor_num; i++) {
        if (dev_node->sensors[i].dms_sensor.sensor_type != sensor_type) {
            continue;
        }

        /* notify the sensor to scan event immediately */
        ret = dms_sensor_event_notify(g_hclge_falut_mgr.dev_id, &dev_node->sensors[i].dms_sensor);
        if (ret != 0) {
            pr_err("hns3: dms sensor event notify failed, ret=%d.\n", ret);
        }
        return ret;
    }

    pr_err("hns3: sensor_type[%d] invalid.\n", sensor_type);
    return -EINVAL;
}

static bool hclge_found_excep_id(u32 excep_id, u8 *module_id, struct hclge_fault_info **fault_info_ptr)
{
    u16 size, i;

    size = sizeof(g_hclge_fault_infos) / sizeof(struct hclge_fault_info);

    for (i = 0; i < size; i++) {
        *fault_info_ptr = &g_hclge_fault_infos[i];
        if (excep_id == (*fault_info_ptr)->excep_id) {
            *module_id = (*fault_info_ptr)->event.module_id;
            return true;
        }
    }

    return false;
}

static int hclge_get_node_id_by_module_id(u8 module_id, u32 excep_id, bool is_recovery)
{
    switch (module_id) {
        case HISI_MODULE_NIC:
            if (excep_id == RAS_EVENT_EXCEP_ID(LINK_ERROR, SOC_SAFETY_LINK_STATE_CHG)) {
                return HCLGE_NODE_NIC_PORT;
            } else {
                return HCLGE_NODE_NIC;
            }
        case HISI_MODULE_ROCE:
            return HCLGE_NODE_ROCE;
        case HISI_MODULE_DISP:
            if (is_recovery) {
                pr_info("hns3: nic_dispatch not support self-recovery");
                return -1;
            } else {
                return HCLGE_NODE_NIC_DISP;
            }
        default:
            pr_err("hns3: report an invalid fault module, module_id = %d\n", module_id);
            return -1;
        }
}

void hclge_fault_report_excep(u32 dev_id, u32 excep_id)
{
    struct hclge_fault_info *fault_info;
    u8 module_id;
    int node_id;
    int ret;

    if (hclge_found_excep_id(excep_id, &module_id, &fault_info)) {
        node_id = hclge_get_node_id_by_module_id(module_id, excep_id, false);
        if (node_id == -1) {
            return;
        }

        ret = dfm_add_event(&g_hclge_falut_mgr, node_id, &fault_info->event);
        if ((ret != 0) && (ret != -EEXIST)) {
            pr_err("hns3: report excep_id[0x%x] failed, ret = %d\n", excep_id, ret);
            return;
        }

        ret = hclge_notify_sensor_scan(node_id, fault_info->event.sensor_type);
        if (ret != 0) {
            pr_err("hns3: notify excep_id[0x%x] failed, ret = %d\n", excep_id, ret);
        }
        return;
    }

    pr_err("hns3: report excep failed, excep_id[0x%x] not found\n", excep_id);
    return;
}
EXPORT_SYMBOL(hclge_fault_report_excep);

void hclge_fault_report_recovery_excep(u32 dev_id, u32 excep_id)
{
    struct hclge_fault_info *fault_info;
    u8 module_id;
    int node_id;
    int ret;

    if (hclge_found_excep_id(excep_id, &module_id, &fault_info)) {
        node_id = hclge_get_node_id_by_module_id(module_id, excep_id, true);
        if (node_id == -1) {
            return;
        }

        ret = dfm_remove_event(&g_hclge_falut_mgr, node_id, &fault_info->event);
        if ((ret != 0) && (ret != -EEXIST)) {
            pr_err("hns3: dfm_remove_event failed, ret = %d.\n", ret);
        }
        return;
    }

    pr_err("hns3: report recovery excep failed, excep_id[0x%x] not found\n", excep_id);
    return;
}
EXPORT_SYMBOL(hclge_fault_report_recovery_excep);

void hclge_link_down_fault_report(struct hclge_dev *hdev, bool flag)
{
    u32 excep_id;
    u32 port_id = hdev->pf_id;

    if (port_id > MAX_PORT_ID) {
        pr_err("hns3: param error, port_id[%u] is invalid, max port num is %u.\n", port_id, MAX_PORT_ID);
        return;
    }

    excep_id = RAS_EVENT_EXCEP_ID(LINK_ERROR, SOC_SAFETY_LINK_STATE_CHG);
    if (flag) {
        hclge_fault_report_excep(hdev->pf_id, excep_id);
    } else {
        hclge_fault_report_recovery_excep(hdev->pf_id, excep_id);
    }
}
