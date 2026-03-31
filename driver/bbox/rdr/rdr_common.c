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

#include "rdr_common.h"

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/notifier.h>
#include <linux/suspend.h>
#include <linux/atomic.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/securec.h>

#include "bootparam/bbox_boot_param.h"
#include "common/bbox_print.h"
#include "common/bbox_common.h"
#include "config/bbox_config.h"
#include "device/bbox_pub.h"
#include "rdr/rdr_field_core.h"
#include "bbox_inner.h"

STATIC const struct reboot_reason_map g_reboot_reason_map[] = {
    {"DEVICE_COLDBOOT", (u8)DEVICE_COLDBOOT},
    {"AP_S_COLDBOOT", (u8)DEVICE_COLDBOOT},
    {"BIOS_EXCEPTION", (u8)BIOS_EXCEPTION},
    {"BIOS_S_EXCEPTION", (u8)BIOS_EXCEPTION},
    {"ABNORMAL_EXCEPTION", (u8)ABNORMAL_EXCEPTION},
    {"AP_S_ABNORMAL", (u8)ABNORMAL_EXCEPTION},
    {"TSENSOR_EXCEPTION", (u8)TSENSOR_EXCEPTION},
    {"AP_S_TSENSOR", (u8)TSENSOR_EXCEPTION},
    {"PMU_EXCEPTION", (u8)PMU_EXCEPTION},
    {"AP_S_PMU", (u8)PMU_EXCEPTION},
    {"DDR_FATAL_EXCEPTION", (u8)DDR_FATAL_EXCEPTION},
    {"AP_S_DDR_FATAL", (u8)DDR_FATAL_EXCEPTION},
    {"OS_PANIC", (u8)OS_PANIC},
    {"AP_S_PANIC", (u8)OS_PANIC},
    {"OS_OOM", (u8)OS_OOM},
    {"OS_COMM", (u8)OS_COMM},
    {"OS_HDC", (u8)OS_COMM},
    {"AP_S_HDC", (u8)OS_COMM},
    {"STARTUP_EXCEPTION", (u8)STARTUP_EXCEPTION},
    {"STARTUP_S_EXCEPTION", (u8)STARTUP_EXCEPTION},
    {"HEARTBEAT_EXCEPTION", (u8)HEARTBEAT_EXCEPTION},
    {"HEARTBEAT_S_EXCEPTION", (u8)HEARTBEAT_EXCEPTION},
    {"RUN_EXCEPTION", (u8)RUN_EXCEPTION},
    {"LPM_EXCEPTION", (u8)LPM_EXCEPTION},
    {"LPM3_S_EXCEPTION", (u8)LPM_EXCEPTION},
    {"TS_EXCEPTION", (u8)TS_EXCEPTION},
    {"TS_S_EXCEPTION", (u8)TS_EXCEPTION},
    {"MICROWATT_EXCEPTION", (u8)MICROWATT_EXCEPTION},
    {"DVPP_EXCEPTION", (u8)DVPP_EXCEPTION},
    {"DVPP_S_EXCEPTION", (u8)DVPP_EXCEPTION},
    {"DRIVER_EXCEPTION", (u8)DRIVER_EXCEPTION},
    {"DRIVER_S_EXCEPTION", (u8)DRIVER_EXCEPTION},
    {"TEE_EXCEPTION", (u8)TEE_EXCEPTION},
    {"TEE_S_EXCEPTION", (u8)TEE_EXCEPTION},
    {"TOOLCHAIN_EXCEPTION", (u8)TOOLCHAIN_EXCEPTION},
    {"LPFW_EXCEPTION", (u8)LPFW_EXCEPTION},
    {"LPFW_S_EXCEPTION", (u8)LPFW_EXCEPTION},
    {"NETWORK_EXCEPTION", (u8)NETWORK_EXCEPTION},
    {"NETWORK_S_EXCEPTION", (u8)NETWORK_EXCEPTION},
    {"HSM_EXCEPTION", (u8)HSM_EXCEPTION},
    {"ATF_EXCEPTION", (u8)ATF_EXCEPTION},
    {"SAFETYISLAND_EXCEPTION", (u8)SAFETYISLAND_EXCEPTION},
    {"DSS_EXCEPTION", (u8)DSS_EXCEPTION},
    {"COMISOLATOR_EXCEPTION", (u8)COMISOLATOR_EXCEPTION},
    {"SD_EXCEPTION", (u8)SD_EXCEPTION},
    {"DP_EXCEPTION", (u8)DP_EXCEPTION},
    {"HDR_EXCEPTION", (u8)HDR_EXCEPTION},
    {"DEVICE_LOAD_TIMEOUT", (u8)DEVICE_LTO_EXCEPTION},
    {"DEVICE_LTO_EXCEPTION", (u8)DEVICE_LTO_EXCEPTION},
    {"DEVICE_HEAT_BEAT_LOST", (u8)DEVICE_HBL_EXCEPTION},
    {"DEVICE_HBL_EXCEPTION", (u8)DEVICE_HBL_EXCEPTION},
    {"BOOT_DOT_INFO", (u8)BOOT_DOT_INFO},
    {"SUSPEND_FAIL", (u8)SUSPEND_FAIL},
    {"RESUME_FAIL", (u8)RESUME_FAIL},
};

STATIC u32 get_reboot_reason_map_size(void)
{
    return (u32)(sizeof(g_reboot_reason_map) / sizeof(g_reboot_reason_map[0]));
}

/*
 * @brief       : get exception type for exception type str.
 * @param [in]  : char *reason        exception type str
 * @return      : exception type
 */
u8 rdr_get_etype(const char *reason)
{
    u32 i;
    BB_CHECK_PTR(reason, return BBOX_EXCEPTION_REASON_INVALID, "invalid param, reason is NULL.\n");
    for (i = 0; i < get_reboot_reason_map_size(); i++) {
        if (strcmp(g_reboot_reason_map[i].name, reason) == 0) {
            return g_reboot_reason_map[i].num;
        }
    }

    return BBOX_EXCEPTION_REASON_INVALID;
}

STATIC struct module_core_map g_rdr_module_core[] = {
    {"DRIVER",       BBOX_DRIVER},
    {"AP",           BBOX_OS},
    {"TS",           BBOX_TS},
    {"DVPP",         BBOX_DVPP},
    {"AIPP",         BBOX_AIPP},
    {"LPM",          BBOX_LPM},
    {"MICROWATT",    BBOX_MICROWATT},
    {"TOOLCHAIN",    BBOX_TOOLCHAIN},
    {"BIOS",         BBOX_BIOS},
    {"TEEOS",        BBOX_TEEOS},
    {"LPFW",         BBOX_LPFW},
    {"NETWORK",      BBOX_NETWORK},
    {"TF",           BBOX_TF}, // TF for interal use(dts/acpi), ATF for public
    {"HSM",          BBOX_HSM},
    {"ISP",          BBOX_ISP},
    {"SAFETYISLAND", BBOX_SAFETYISLAND},
    {"DSS",          BBOX_DSS},
    {"CLUSTER",      BBOX_DSS},
    {"COMISOLATOR",  BBOX_COMISOLATOR},
    {"AOS-SD",       BBOX_AOS_SD},
    {"AOS-DP",       BBOX_AOS_DP},
    {"AOS-LINUX",    BBOX_AOS_LINUX},
    {"AOS-CORE",     BBOX_AOS_CORE},
    {"DP",           BBOX_AOS_DP}, // mini\cloud\dc uded for kbox
    {"IMU",          BBOX_IMU},
};

STATIC u32 get_module_core_map_size(void)
{
    return (u32)(sizeof(g_rdr_module_core) / sizeof(g_rdr_module_core[0]));
}

/*
 * @brief       : get exception core str for core id.
 * @param [in]  : u8 coreid
 * @return      : core name str or UNDEF
 */
const char *rdr_get_exception_core(u8 coreid)
{
    u32 i;
    for (i = 0; i < get_module_core_map_size(); i++) {
        if (coreid == g_rdr_module_core[i].coreid) {
            return g_rdr_module_core[i].name;
        }
    }

    return "UNDEF";
}

/*
 * @brief       : get exception core id.
 * @param [in]  : const char *name      module name
 * @return      : core id.
 */
u8 rdr_get_exception_coreid(const char *name)
{
    u32 i;
    BB_CHECK_PTR(name, return BBOX_UNDEF, "invalid param, name is NULL.\n");
    for (i = 0; i < get_module_core_map_size(); i++) {
        if (strcmp(g_rdr_module_core[i].name, name) == 0) {
            return g_rdr_module_core[i].coreid;
        }
    }

    return BBOX_UNDEF;
}

const char *rdr_get_model_name(u32 excepid)
{
    u8 coreid = bbox_excepid_get_coreid(excepid);
    if (bbox_check_coreid_valid(coreid) == BBOX_SUCCESS) {
        return rdr_get_exception_core(coreid);
    }

    return "UNDEF";
}

STATIC atomic_t g_bb_in_saving = ATOMIC_INIT(BBOX_FALSE);
STATIC atomic_t g_bb_in_suspend = ATOMIC_INIT(BBOX_FALSE);

s32 rdr_get_suspend_state(void)
{
    return atomic_read(&g_bb_in_suspend);
}

void rdr_set_saving_state(s32 state)
{
    atomic_set(&g_bb_in_saving, state);
}

#define ATOMIC_MSLEEP_NUM   1000
STATIC struct notifier_block g_bb_suspend_notifier;
STATIC s32 bb_suspend_nb(struct notifier_block *notifier, bbox_event_t event, bbox_arg_t *arg)
{
    s32 ret = NOTIFY_OK;
    UNUSED(notifier);
    UNUSED(arg);

    switch (event) {
        case PM_POST_HIBERNATION:
        case PM_POST_SUSPEND:
            atomic_set(&g_bb_in_suspend, BBOX_FALSE);
            break;
        case PM_HIBERNATION_PREPARE:
        case PM_SUSPEND_PREPARE:
            atomic_set(&g_bb_in_suspend, BBOX_TRUE);
            while (1) {
                if (atomic_read(&g_bb_in_saving)) {
                    msleep(ATOMIC_MSLEEP_NUM);
                } else {
                    break;
                }
            }
            break;
        default:
            ret = NOTIFY_DONE;
            break;
    }
    return ret;
}

s32 rdr_common_init(void)
{
#if (defined BBOX_SOC_PLATFORM_MDC) && (!defined BBOX_UT)
    s32 ret = bbox_get_ctrlcpu_mask();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("Get ctrl cpu mask failed.(ret=%d)\n", ret);
    }
#endif
    // Register to get PM events
    g_bb_suspend_notifier.notifier_call = bb_suspend_nb;
    g_bb_suspend_notifier.priority = -1;
    if (register_pm_notifier(&g_bb_suspend_notifier) != 0) {
        BB_PRINT_ERR("%s: Failed to register for PM events\n", __func__);
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

void rdr_common_exit(void)
{
    (void)unregister_pm_notifier(&g_bb_suspend_notifier);
}

/*
 * @brief       : map bbox physics address to virtual address
 * @param [in]  : u32 devid             device id
 * @param [in]  : phys_addr_t paddr     physics address
 * @param [in]  : size_t size           buffer size
 * @return      : virtual address
 */
void *rdr_map(u32 devid, phys_addr_t paddr, size_t size)
{
    s32 ret;
    u64 reserved_addr, reserved_size;

    BB_CHECK_EXP_ACT(size == 0, return NULL, "invalid size.\n");
    BB_CHECK_EXP_ACT(devid >= bbox_get_device_num(), return NULL, "invalid device id: %u.\n", devid);

    ret = bbox_config_get_rdr(&reserved_addr, &reserved_size);
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return NULL, "get rdr phymem addr&size failed.\n");

    if ((paddr < reserved_addr) ||
        ((paddr + size) > (reserved_addr + reserved_size))) {
        BB_PRINT_ERR("error dev[%u] BBox memory. Map size[0x%lx], bbox reserved size[0x%llx]\n",
                     devid, size, reserved_size);
        return NULL;
    }

    return ioremap_wc((phys_addr_t)bbox_get_device_addr(devid, paddr), size);
}

/*
 * @brief       : unmap virtual address
 * @param [in]  : const void *vaddr     virtual address
 * @return      : NA
 */
void rdr_unmap(const void *vaddr)
{
    if (vaddr != NULL) {
        iounmap((void __iomem *)vaddr);
    }
}

