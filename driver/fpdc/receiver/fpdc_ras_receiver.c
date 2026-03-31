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

#include <linux/notifier.h>
#include <linux/securec.h>

#ifdef AOS_LLVM_BUILD
#  include <linux/acpi/ghes.h>
#else
#  include <acpi/ghes.h>
#endif
#include "fpdc_define.h"
#include "distribute/fpdc_distribute_notify.h"
#include "fpdc_ras_receiver.h"
#include "log_restrain/log_restrain.h"
#include "drv_ras_common.h"
#include "drv_systime.h"

/*
 * Description: get node type(DDR or HBM or HHA) by sub id
 * History£º2021-11-20
 */
STATIC int get_hha_node_type(unsigned int sub_id, DMS_DEVICE_NODE_TYPE *node_type)
{
    if (sub_id == 0) { /* HHA_DDR:0 */
        *node_type = DMS_DEV_TYPE_DDRA;
    } else if ((sub_id >= 0x01) && (sub_id <= 0x08)) { /* HBM_HHA0~HHA7:0x01~0x08 */
        *node_type = DMS_DEV_TYPE_HBMA;
    } else {
        fpdc_err("Invalid sub id. (sub_id=%u)\n", sub_id);
        return -EINVAL;
    }
    return 0;
}

STATIC int get_mem_node_type(unsigned int sub_id, DMS_DEVICE_NODE_TYPE *node_type)
{
    if ((sub_id == ECC_MODULE_DDRC0) || (sub_id == ECC_MODULE_DDRC1)) {
        *node_type = DMS_DEV_TYPE_DDR;
    } else if ((sub_id >= ECC_MODULE_HBM_MIN) && (sub_id <= ECC_MODULE_HBM_MAX)) {
        *node_type = DMS_DEV_TYPE_HBM;
    } else {
        fpdc_err("Invalid sub id. (sub_id=%u)\n", sub_id);
        return -EINVAL;
    }
    return 0;
}

static const struct {
    unsigned char module_id;
    DMS_DEVICE_NODE_TYPE node_type;
} g_oem2_module_node_type_tbl[] = {
    {RAS_SMMU_MODULE, DMS_DEV_TYPE_SLLC},
    {RAS_HHA_MODULE, DMS_DEV_TYPE_HBM},
    {RAS_PA_MODULE, DMS_DEV_TYPE_NIC},
    {RAS_HLLC_MODULE, DMS_DEV_TYPE_NIC},
    {RAS_L2BUFF_MODULE, DMS_DEV_TYPE_L2BUF}
};

STATIC int get_node_type_by_module_id(unsigned char module_id, DMS_DEVICE_NODE_TYPE *node_type)
{
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(g_oem2_module_node_type_tbl); i++) {
        if (g_oem2_module_node_type_tbl[i].module_id == module_id) {
            *node_type = g_oem2_module_node_type_tbl[i].node_type;
            return 0;
        }
    }

    fpdc_err("Unrecognized module id. (module_id=%u)\n", module_id);
    return -ENFILE;
}

/*
 * Description: parse location of section type memory(memory error with address)
 * History£º2021-11-20
 */
STATIC int ras_parse_mem(const void *data, struct notify_data *pnotify)
{
    const struct cper_sec_mem_err *mem_err = (const struct cper_sec_mem_err *)data;
    int ret;

    if (!(mem_err->validation_bits & CPER_MEM_VALID_MODULE)) {
        fpdc_err("Validation bits not include module. (bits=0x%llx; module=%d)\n", mem_err->validation_bits,
            CPER_MEM_VALID_MODULE);
        return -ESPIPE;
    }
    ret = get_mem_node_type(mem_err->module, &pnotify->node_type);
    if (ret) {
        return ret;
    }

    pnotify->chip_id = mem_err->device;
    pnotify->sub_id = mem_err->module; /* ddr0,ddr1,hbm0,hbm1,hbm2... */
    pnotify->origin_data = mem_err;
    pnotify->data_len = sizeof(*mem_err);

    return fpdc_distribute_notify(pnotify);
}

/*
 * Description: parse location of section type oem2
 * History£º2021-11-20
 */
STATIC int ras_parse_oem2(const void *data, struct notify_data *pnotify)
{
    const struct sec_oem_error *oem_err = (const struct sec_oem_error *)data;
    int ret;

    if (!(oem_err->valid_fields & HISI_OEM_VALID_MODULE_ID)) {
        fpdc_err("Validation bits not include module. (bits=0x%x; module=0x%lx)\n", oem_err->valid_fields,
            HISI_OEM_VALID_MODULE_ID);
        return -ESPIPE;
    }

    if (oem_err->module_id == RAS_MEMORY_MODULE) { /* memory error without address */
        ret = get_mem_node_type(oem_err->submodule_id, &pnotify->node_type);
        if (ret) {
            return ret;
        }
    } else if (oem_err->module_id == RAS_HHA_MODULE) {
        ret = get_hha_node_type(oem_err->submodule_id, &pnotify->node_type);
        if (ret) {
            return ret;
        }
    } else {
        ret = get_node_type_by_module_id(oem_err->module_id, &pnotify->node_type);
        if (ret) {
            return ret;
        }
    }

    pnotify->chip_id = oem_err->socket_id;
    pnotify->sub_id =oem_err->submodule_id;
    pnotify->origin_data = oem_err;
    pnotify->data_len = sizeof(*oem_err);

    return fpdc_distribute_notify(pnotify);
}

#ifndef AOS_LLVM_BUILD
/*
 * Description: parse location of section type ts, use same struct as oem2
 * History£º2021-11-22
 */
STATIC int ras_parse_oem2_ts(const void *data, struct notify_data *pnotify)
{
    const struct sec_oem_error *oem_err = (const struct sec_oem_error *)data;

    if (!(oem_err->valid_fields & HISI_OEM_VALID_MODULE_ID)) {
        fpdc_err("Validation bits not include module. (bits=0x%x; module=0x%lx)\n", oem_err->valid_fields,
            HISI_OEM_VALID_MODULE_ID);
        return -ESPIPE;
    }

    if (oem_err->module_id == RAS_TS_MODULE) {
        pnotify->node_type = DMS_DEV_TYPE_TSCPU;
    } else {
        fpdc_err("Invalid module id. (module_id=%u)\n", oem_err->module_id);
        return -EINVAL;
    }

    pnotify->chip_id = oem_err->socket_id;
    pnotify->sub_id =oem_err->submodule_id;
    pnotify->origin_data = oem_err;
    pnotify->data_len = sizeof(*oem_err);

    return fpdc_distribute_notify(pnotify);
}
#endif

STATIC int ras_parse_pcie_local(const void *data, struct notify_data *pnotify)
{
    const struct cper_sec_pcie_local *err = (const struct cper_sec_pcie_local *)data;

    if (!(err->valid_fields & HISI_PCIE_LOCAL_VALID_ERROR_TYPE) ||
        !(err->valid_fields & HISI_PCIE_LOCAL_VALID_SUB_MODULE_ID)) {
        fpdc_err("PCIE local ras validation bits not include error type or sub module id. (bits=0x%llx)\n",
            err->valid_fields);
        return -ESPIPE;
    }

    if (err->submodule_id > PCIE_LOCAL_MODULE_SDI) {
        fpdc_err("Invalid sub module id. (submodule_id=%u)\n", err->submodule_id);
        return -EINVAL;
    }

    pnotify->node_type = DMS_DEV_TYPE_PCIE;
    pnotify->chip_id = err->socket_id;
    pnotify->sub_id = err->submodule_id;
    pnotify->origin_data = err;
    pnotify->data_len = sizeof(*err);

    return fpdc_distribute_notify(pnotify);
}

static const struct {
    unsigned char cache_level;
    DMS_DEVICE_NODE_TYPE node_type;
} g_cache_level_node_type_tbl[] = {
    {4, DMS_DEV_TYPE_L3D},
    {5, DMS_DEV_TYPE_L3T}
};

STATIC int get_node_type_by_cache_level(unsigned char level, DMS_DEVICE_NODE_TYPE * node_type)
{
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(g_cache_level_node_type_tbl); i++) {
        if (g_cache_level_node_type_tbl[i].cache_level == level) {
            *node_type = g_cache_level_node_type_tbl[i].node_type;
            return 0;
        }
    }

    return -ENFILE;
}

STATIC int ras_parse_arm_err_info(unsigned int type, unsigned long long error_info,
    struct notify_data *pnotify)
{
    unsigned char level;
    int ret;

    if (!(error_info & CPER_ARM_ERR_VALID_LEVEL)) {
        fpdc_err("Level in error info is not valid.\n");
        return -ESPIPE;
    }
    level = ((error_info >> CPER_ARM_ERR_LEVEL_SHIFT) & CPER_ARM_ERR_LEVEL_MASK);

    if (type != CPER_ARM_CACHE_ERROR) {
        fpdc_err("Not support this arm error type. (type=%u)\n", type);
        return -EINVAL;
    }
    ret = get_node_type_by_cache_level(level, &pnotify->node_type);
    if (ret) {
        pnotify->node_type = DMS_DEV_TYPE_CPU_CORE;
    }

    return fpdc_distribute_notify(pnotify);
}

STATIC int get_chip_id(const struct cper_sec_proc_arm *arm_err, int len, unsigned int *chip_id)
{
    int i;
    struct cper_arm_err_info *err_info = NULL;
    struct cper_arm_ctx_info *ctx_info = NULL;
    struct vendor_specific_error_info *vendor_info = NULL;

    err_info = (struct cper_arm_err_info *)(arm_err + 1);
    ctx_info = (struct cper_arm_ctx_info *)(err_info + arm_err->err_info_num);

    for (i = 0; i < arm_err->context_info_num; i++) {
        int size = sizeof(*ctx_info) + ctx_info->size;

        if (len < size) {
            fpdc_err("len is less than context size. (len=%d; size=%d)\n", len, size);
            return -EINVAL;
        }

        len -= size;
        ctx_info = (struct cper_arm_ctx_info *)((long)ctx_info + size);
    }

    if ((unsigned long)len < sizeof(struct vendor_specific_error_info)) {
        fpdc_warn("len is less than vendor size. (len=%d; vendor_info=%lu)\n",
            len, sizeof(struct vendor_specific_error_info));
        return 0;
    }
    vendor_info = (struct vendor_specific_error_info *)ctx_info;

    if (vendor_info->oem_valid_flag & 0x01) { /* bit0 -> 0:old version 1:new version */
        *chip_id = vendor_info->oem_socket_id;
    }
    return 0;
}

/*
 * Description: parse location of section type arm processor
 * History£º2021-11-20
 */
STATIC int ras_parse_arm(const void *data, struct notify_data *pnotify)
{
    const struct cper_sec_proc_arm *arm_err = (const struct cper_sec_proc_arm *)data;
    int i, len;
    struct cper_arm_err_info *err_info = NULL;
    int ret;

    fpdc_info("Receive arm ras info. (MIDR=0x%016llx; err_info_num=%u; context_info_num=%u)\n", arm_err->midr,
        arm_err->err_info_num, arm_err->context_info_num);

    len = arm_err->section_length - (sizeof(*arm_err) + arm_err->err_info_num * (sizeof(*err_info)));
    if (len < 0) {
        fpdc_err("Section length is too small. (section_length=%u)\n", arm_err->section_length);
        return -EINVAL;
    }

    pnotify->origin_data = arm_err;
    pnotify->data_len = sizeof(*arm_err);

    if (arm_err->validation_bits & CPER_ARM_VALID_MPIDR) {
        pnotify->chip_id = ((arm_err->mpidr >> CPER_ARM_MPIDR_SOCKETID_SHIFT) & CPER_ARM_MPIDR_SOCKETID_MASK);
        pnotify->sub_id = arm_err->mpidr;
    } else {
        fpdc_err("MPIDR in arm ras error is not valid.\n");
        return -ESPIPE;
    }

    err_info = (struct cper_arm_err_info *)(arm_err + 1);

    ret = get_chip_id(arm_err, len, &pnotify->chip_id);
    if (ret != 0) {
        fpdc_err("get_chip_id is fail. (ret=%d)\n", ret);
        return ret;
    }

    ret = -ENODEV;
    for (i = 0; i < arm_err->err_info_num; i++) {
        fpdc_info("Parse arm ras info. (i=%d; type=%u; vali_bits=%u; err_info=0x%016llx; err_num=%d)\n", i,
            err_info->type, err_info->validation_bits, err_info->error_info, err_info->multiple_error + 1);

        pnotify->arm_error_idx = i;
        if (err_info->validation_bits & CPER_ARM_INFO_VALID_ERR_INFO) {
            if (ras_parse_arm_err_info(err_info->type, err_info->error_info, pnotify) == 0) {
                ret = 0;
            }
        }
        err_info += 1;
    }

    return ret;
}

static const struct {
    HISI_SUBSYS_ID subsys_id;
    DMS_DEVICE_NODE_TYPE node_type;
}  g_hisi_subsys_node_type_tbl[]  = {
    { HISI_SUBSYS_HACSUB, DMS_DEV_TYPE_HAC_SUBSYS},
    { HISI_SUBSYS_AOSUB, DMS_DEV_TYPE_AO_SUBSYS},
    { HISI_SUBSYS_PERISUB, DMS_DEV_TYPE_PERI_SUBSYS},
    { HISI_SUBSYS_SILSUB, DMS_DEV_TYPE_SAFETYISLAND},
    { HISI_SUBSYS_AICORESUB, DMS_DEV_TYPE_AIC},
    { HISI_SUBSYS_AIVECTORSUB, DMS_DEV_TYPE_AIV},
    { HISI_SUBSYS_GPUSUB, DMS_DEV_TYPE_GPU},
    { HISI_SUBSYS_TSSUB, DMS_DEV_TYPE_TS},
    { HISI_SUBSYS_IOSUB, DMS_DEV_TYPE_IO_SUBSYS},
    { HISI_SUBSYS_ISPSUB, DMS_DEV_TYPE_ISP_SUB_SYS},
    { HISI_SUBSYS_MEDIASUB, DMS_DEV_TYPE_MEDIA_SUB_SYS},
    { HISI_SUBSYS_NICSUB, DMS_DEV_TYPE_NIC},
    { HISI_SUBSYS_PCIESUB, DMS_DEV_TYPE_PCIE},
    { HISI_SUBSYS_DSASUB, DMS_DEV_TYPE_DSA},
    { HISI_SUBSYS_DVPPSUB, DMS_DEV_TYPE_DVPP}
};

static const struct {
    HISI_MODULE_ID module_id;
    DMS_DEVICE_NODE_TYPE node_type;
}  g_hisi_module_node_type_tbl[]  = {
    {HISI_MODULE_MN, DMS_DEV_TYPE_MN},
    {HISI_MODULE_PLL, DMS_DEV_TYPE_LPM},
    {HISI_MODULE_SLLC, DMS_DEV_TYPE_SLLC},
    {HISI_MODULE_AA, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_SIOE, DMS_DEV_TYPE_SIOE},
    {HISI_MODULE_POE, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_CPA, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_DISP, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_GIC, DMS_DEV_TYPE_GIC},
    {HISI_MODULE_ITS, DMS_DEV_TYPE_ITS},
    {HISI_MODULE_AVSBUS, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_CS, DMS_DEV_TYPE_CS},
    {HISI_MODULE_PPU, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_SMMU, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_PA, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_HLLC, DMS_DEV_TYPE_HCCS},
    {HISI_MODULE_DDRC, DMS_DEV_TYPE_DDR},
    {HISI_MODULE_L3TAG, DMS_DEV_TYPE_L3T},
    {HISI_MODULE_L3DATA, DMS_DEV_TYPE_L3D},
    {HISI_MODULE_PCS, DMS_DEV_TYPE_HCCS},
#ifdef CFG_SOC_PLATFORM_MINIV3
    {HISI_MODULE_MATA, DMS_DEV_TYPE_DDRA},
#else
    {HISI_MODULE_MATA, DMS_DEV_TYPE_HBMA},
#endif
    {HISI_MODULE_PCIE_LOCAL, DMS_DEV_TYPE_PCIE},
    {HISI_MODULE_SAS, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_SATA, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_NIC, DMS_DEV_TYPE_NIC},
    {HISI_MODULE_ROCE, DMS_DEV_TYPE_ROCE},
    {HISI_MODULE_USB, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_ZIP, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_HPRE, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_SEC, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_RDE, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_MEE, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_L4D, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_SCH, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_DJTAG, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_CFG_BUS, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_R52_SENSORHUB, DMS_DEV_TYPE_SENSORHUB},
    {HISI_MODULE_R52_LP, DMS_DEV_TYPE_LPM},
    {HISI_MODULE_CPU_TS,  DMS_DEV_TYPE_TSCPU},
    {HISI_MODULE_TSENSOR, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_L2BUFFER, DMS_DEV_TYPE_L2BUF},
    {HISI_MODULE_AICORE, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_AIVECTOR, DMS_DEV_TYPE_MAX},
    {HISI_MODULE_RING_BRIDGE, DMS_DEV_TYPE_RBRG},
    {HISI_MODULE_HILINK, DMS_DEV_TYPE_HILINK},
    {HISI_MODULE_HBM, DMS_DEV_TYPE_HBM}
};

static DMS_DEVICE_NODE_TYPE get_node_type_from_hisi_info(const hisi_common_error_info *hisi)
{
    DMS_DEVICE_NODE_TYPE node_type = DMS_DEV_TYPE_MAX;
    unsigned int i;
    u8 sub_sys_id = hisi->sub_sys_id;
    u8 module_id = hisi->module_id;

    for (i = 0; i < ARRAY_SIZE(g_hisi_subsys_node_type_tbl); i++) {
        if (g_hisi_subsys_node_type_tbl[i].subsys_id == sub_sys_id) {
            node_type = g_hisi_subsys_node_type_tbl[i].node_type;
            break;
        }
    }

    if (node_type != DMS_DEV_TYPE_MAX) {
        return node_type;
    }

    for (i = 0; i < ARRAY_SIZE(g_hisi_module_node_type_tbl); i++) {
        if (g_hisi_module_node_type_tbl[i].module_id == module_id) {
            node_type = g_hisi_module_node_type_tbl[i].node_type;
            break;
        }
    }
    return node_type;
}

STATIC void ras_print_hisi_common_info(const hisi_common_error_info *hisi_error)
{
    u8 num = (u8)(hisi_error->register_array_size / sizeof(u32));
    char registers[MAX_REGISTER_ARRAY_SIZE * 12];
    u32 i;
    int ret;

    if (num > MAX_REGISTER_ARRAY_SIZE) {
        num = MAX_REGISTER_ARRAY_SIZE;
    }

    ret = memset_s(registers, sizeof(registers), 0, sizeof(registers));
    if (ret != 0) {
        fpdc_warn("memset_s failed.(ret=%d)\n", ret);
        return;
    }

    for (i = 0; i < num; ++i) {
        if (i == 0) {
            ret = sprintf_s(registers, sizeof(registers), "0x%x", hisi_error->register_array[i]);
        } else {
            ret = sprintf_s(registers, sizeof(registers), "%s,0x%x", registers, hisi_error->register_array[i]);
        }
        if (ret <= 0) {
            fpdc_warn("sprintf_s failed. [%d]->[0x%x]\n", i, hisi_error->register_array[i]);
        }
    }

    fpdc_info("received a safety event. (subsys id=%u, module id=%u, registers=%s)\n",
        hisi_error->sub_sys_id, hisi_error->module_id, registers);
}

STATIC int ras_parse_hisi_common(const void *data, struct notify_data *pnotify)
{
    const hisi_common_error_info *hisi_error = (const hisi_common_error_info *)data;
    int ret = 0;

    if (hisi_error == NULL || pnotify == NULL) {
        fpdc_err("Input parameter is invalid .\n");
        return -EINVAL;
    }
    pnotify->node_type = get_node_type_from_hisi_info(hisi_error);
#ifdef CFG_SOC_PLATFORM_MDC
    pnotify->chip_id = hisi_error->socket_id;
#else
    pnotify->chip_id = hisi_error->totem_id;
#endif
    pnotify->sub_id = hisi_error->sub_module_id;
    pnotify->origin_data = hisi_error;
    pnotify->data_len = sizeof(*hisi_error);
    pnotify->arm_error_idx  = 0;

    ret = fpdc_distribute_notify(pnotify);

    /* After a fault is rectified, the register array is printed to facilitate fault locating */
    ras_print_hisi_common_info(hisi_error);

    return ret;
}

static const struct {
    guid_t sec_type;
    int (*func)(const void *data, struct notify_data *pnotify);
} g_ras_parse_tbl[] = {
    {
        .sec_type = CPER_SEC_PLATFORM_MEM,
        .func = ras_parse_mem,
    },
    {
        .sec_type = CPER_SEC_HISI_OEM_2,
        .func = ras_parse_oem2,
    },
    {
        .sec_type = CPER_SEC_PROC_ARM,
        .func = ras_parse_arm,
    },
#ifndef AOS_LLVM_BUILD
    {
        .sec_type = CPER_SEC_TS_CORE,
        .func = ras_parse_oem2_ts,
    },
#endif
    {
        .sec_type = CPER_SEC_HISI_PCIE_LOCAL,
        .func = ras_parse_pcie_local,
    },
    {
        .sec_type = CPER_SEC_HISI_COMMON,
        .func = ras_parse_hisi_common,
    }
};

#define GUID_BYTE_OFFSET0 0
#define GUID_BYTE_OFFSET1 1
#define GUID_BYTE_OFFSET2 2
#define GUID_BYTE_OFFSET3 3

#define RAS_PROC_LOG_PERIOD 60
#define RAS_PROC_LOG_MAX    10

static inline bool fpdc_guid_equal(const guid_t *u1, const guid_t *u2)
{
    return memcmp(u1, u2, sizeof(guid_t)) == 0;
}

int ras_err_notifier_callback(struct notifier_block *nb, unsigned long severity, void *data)
{
    guid_t *sec_type = NULL;
    unsigned int count;
    struct acpi_hest_generic_data *gdata = (struct acpi_hest_generic_data *)data;
    struct notify_data notify_data = {.src_type = FPDC_SRC_RAS};
    const char *handle_ret = "not found handle";
    static struct log_res restrain = {0};

    fpdc_warn("received a ras event. (syscnt=%llu)\n", get_syscnt());

    if ((data == NULL) || (nb == NULL)) {
        fpdc_err("Invalid para, data or nb is NULL. (data=%d; nb=%d)\n", !!data, !!nb);
        return -EINVAL;
    }

    sec_type = (guid_t *)gdata->section_type;
    for (count = 0; count < ARRAY_SIZE(g_ras_parse_tbl); count++) {
        if (!fpdc_guid_equal(sec_type, &g_ras_parse_tbl[count].sec_type)) {
            continue;
        }
        notify_data.section_type = &g_ras_parse_tbl[count].sec_type;
        if (g_ras_parse_tbl[count].func(acpi_hest_get_payload(gdata), &notify_data)) {
            handle_ret = "handle failed";
        } else {
            handle_ret = "handle successfully";
        }
        break;
    }

    if (log_restrain(&restrain, RAS_PROC_LOG_PERIOD, RAS_PROC_LOG_MAX, __func__)) {
        return 0;
    }
    fpdc_event("Receive ras notify, %s. (severity=%lu; section:0x%02x%02x%02x%02x; node_type=0x%x; revision=0x%x)\n",
        handle_ret, severity, gdata->section_type[GUID_BYTE_OFFSET3], gdata->section_type[GUID_BYTE_OFFSET2],
        gdata->section_type[GUID_BYTE_OFFSET1], gdata->section_type[GUID_BYTE_OFFSET0], notify_data.node_type,
        gdata->revision);
    return 0;
}
EXPORT_SYMBOL_UNRELEASE(ras_err_notifier_callback);

static struct notifier_block g_ras_err_notifier_ops = {
    .notifier_call = ras_err_notifier_callback,
};

int fpdc_ras_receiver_init(void)
{
    int err;

    /* call os interface to receiver data */
    err = ghes_register_vendor_record_notifier(&g_ras_err_notifier_ops);
    if (err != 0) {
        fpdc_err("Register ras err notifier fail. (err=%d)\n", err);
        return err;
    }

    fpdc_info("Register ras event notifier success.\n");
    return 0;
}

void fpdc_ras_receiver_exit(void)
{
    ghes_unregister_vendor_record_notifier(&g_ras_err_notifier_ops);
    fpdc_info("Unregister ras event notifier success.\n");
    return;
}

