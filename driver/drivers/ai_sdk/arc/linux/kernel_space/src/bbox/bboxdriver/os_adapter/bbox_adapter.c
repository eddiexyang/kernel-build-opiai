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

#include "bbox_adapter.h"

#include <asm/cacheflush.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/kdebug.h>
#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/printk.h>
#include <linux/securec.h>
#include <linux/version.h>

#include "bbox_adapter_oom.h"
#include "bbox_adapter_snapshot.h"
#include "bootparam/bbox_boot_param.h"
#include "communication/bbox_channel_status.h"
#include "config/bbox_config.h"
#include "rdr/rdr_common.h"
#include "rdr/rdr_dump_core.h"
#include "register/bbox_register.h"
#include "common/bbox_sys_api.h"

#ifdef DEBUG
#define DEAD_WHILE do { } while (0)
#elif defined (BBOX_SOC_PLATFORM_MDC) || defined (BBOX_SOC_PLATFORM_MDC_V11)
#define DEAD_WHILE do { } while (0)
#elif defined (BBOX_SOC_PLATFORM_DC) && defined(BBOX_FEATURE_HELPER)
#define DEAD_WHILE do { } while (0)
#else
#define DEAD_WHILE do { } while (1)
#endif

STATIC struct ap_root *g_bbox_ap_root = NULL;
STATIC u64 g_bbox_adapter_addr = 0;
STATIC bool g_ap_init = false;
STATIC struct bbox_module_result g_ap_module_info;
STATIC DEFINE_MUTEX(g_ap_log_lock);

struct bbox_exception_info g_einfo[] = {
    {   (u32)EXCEPID_AP_PANIC, (u32)EXCEPID_AP_PANIC,
        BBOX_COREID_MASK(BBOX_OS), BBOX_COREID_MASK(BBOX_OS),
        (u8)OS_PANIC, (u8)BBOX_OS, (u8)BBOX_CRITICAL, (u8)BBOX_REBOOT_NOW,
        (u8)BBOX_REENTRANT_DISALLOW, "ap", "os panic", NULL
    },
    {   (u32)EXCEPID_AP_COMM, (u32)EXCEPID_AP_COMM,
        BBOX_COREID_MASK(BBOX_OS), 0,
        (u8)OS_COMM, (u8)BBOX_OS, (u8)BBOX_MAJOR, (u8)BBOX_REBOOT_NO,
        (u8)BBOX_REENTRANT_DISALLOW, "ap", "bbox communication disconnect", NULL
    },
    {   (u32)EXCEPID_AP_OOM, (u32)EXCEPID_AP_OOM,
        BBOX_COREID_MASK(BBOX_OS), 0,
        (u8)OS_OOM, (u8)BBOX_OS, (u8)BBOX_MINOR, (u8)BBOX_REBOOT_NO,
        (u8)BBOX_REENTRANT_ALLOW, "ap", "os oom", NULL
    },
};

/*
 * @brief       : get os exception info by exception type
 * @param [in]  : u32 etype    exception type to find info
 * @return      : exception info
 */
STATIC struct bbox_exception_info *bbox_adapter_get_einfo(u32 etype)
{
    u32 i;
    for (i = 0; i < (sizeof(g_einfo) / sizeof(struct bbox_exception_info)); i++) {
        if (g_einfo[i].e_exce_type == etype) {
            return &g_einfo[i];
        }
    }
    return NULL;
}

/*
 * @brief       : ummap all devices' virtual addresses
 * @return      : NA
 */
STATIC void bbox_adapter_unmap(void)
{
    if (g_bbox_adapter_addr != 0) {
        rdr_unmap((void *)(uintptr_t)g_bbox_adapter_addr);
        g_bbox_adapter_addr = 0;
    }
}

/*
 * @brief       : map physical to virtual add for all devices
 * @return      : <0 failure; ==0 success
 */
STATIC s32 bbox_adapter_map(void)
{
    g_bbox_adapter_addr = (u64)(uintptr_t)rdr_map(DEFAULT_DEVICE_ID,
                                                  g_ap_module_info.log_addr,
                                                  g_ap_module_info.log_len);
    if (g_bbox_adapter_addr == 0) {
        BB_PRINT_ERR("g_bbox_adapter_addr is invalid.\n");
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

/*
 * @brief       : flush log cache
 * @return      : NA
 */
void bbox_adapter_flush_log_cache(void)
{
    const u8 *vaddr = (u8 *)log_buf_addr_get();
    u32 size = log_buf_len_get();
    if ((vaddr != NULL) && (size > 0)) {
        bbox_flush_cache(vaddr, size);
        BB_PRINT_INFO("flush log cache.\n");
    }
}

STATIC void bbox_adapter_save_log(u32 devid, u32 excepid, u8 etype,
                                  u8 coreid, const struct bbox_time *tm)
{
    s32 idx;
    BB_CHECK_PTR(tm, return, "invalid param, time is NULL.\n");

    idx = g_bbox_ap_root->log_info.next_valid_index;
    if (idx < 0 || idx >= AP_LOG_BUFFER_NUM) {
        BB_PRINT_ERR("Invalid param, idx=%d.\n", idx);
        return;
    }
    g_bbox_ap_root->log_info.next_valid_index++;
    g_bbox_ap_root->log_info.next_valid_index %= AP_LOG_BUFFER_NUM;
    if (g_bbox_ap_root->log_info.log_num < AP_LOG_BUFFER_NUM) {
        g_bbox_ap_root->log_info.log_num++;
    }

    g_bbox_ap_root->log_info.log_buffer[idx].devid = devid;
    g_bbox_ap_root->log_info.log_buffer[idx].excepid = excepid;
    g_bbox_ap_root->log_info.log_buffer[idx].etype = etype;
    g_bbox_ap_root->log_info.log_buffer[idx].coreid = coreid;
    bbox_get_date(tm, g_bbox_ap_root->log_info.log_buffer[idx].date, DATATIME_MAXLEN);
}

void bbox_adapter_save_oom_log(u32 devid, u32 excepid, u8 etype, u8 coreid, const struct bbox_time *tm)
{
    mutex_lock(&g_ap_log_lock);
    g_bbox_ap_root->log_info.event_flag |= (u16)EVENT_OOM_TRIGGER;
    bbox_adapter_save_log(devid, excepid, etype, coreid, tm);
    mutex_unlock(&g_ap_log_lock);
}

STATIC void bbox_adapter_save_comm_log(u32 devid, u32 excepid, u8 etype, u8 coreid, const struct bbox_time *tm)
{
    mutex_lock(&g_ap_log_lock);
    bbox_adapter_save_log(devid, excepid, etype, coreid, tm);
    mutex_unlock(&g_ap_log_lock);
}

STATIC void bbox_adapter_save_run_log(u32 devid, u32 excepid, u8 etype, u8 coreid, const struct bbox_time *tm)
{
    mutex_lock(&g_ap_log_lock);
    bbox_adapter_save_log(devid, excepid, etype, coreid, tm);
    mutex_unlock(&g_ap_log_lock);
}

static void bbox_adapter_save_panic_log(u32 devid, u32 excepid, u8 etype, u8 coreid, const struct bbox_time *tm)
{
    bbox_adapter_save_log(devid, excepid, etype, coreid, tm);
}

/*
 * @brief       : print struct ap_root value for debug
 * @return      : NA
 */
STATIC void bbox_adapter_print_info(void)
{
    BB_PRINT_INFO("=================OS INFO================\n");
    BB_PRINT_INFO("version         [0x%x]\n", g_bbox_ap_root->top_head.version);
    BB_PRINT_INFO("devid           [0x%x]\n", g_bbox_ap_root->current_info.devid);
    BB_PRINT_INFO("coreid          [0x%hhx]\n", g_bbox_ap_root->current_info.coreid);
    BB_PRINT_INFO("excepid         [0x%x]\n", g_bbox_ap_root->current_info.excepid);
    BB_PRINT_INFO("etype           [0x%hhx]\n", g_bbox_ap_root->current_info.etype);
}

/*
 * @brief       : check whether specific add exceeds device add limitation
 * @param [in]  : u64 addr    add to check
 * @return      : <0 failure; ==0 success
 */
STATIC s32 bbox_adapter_check_vaddr(u64 addr)
{
    u64 min_addr = g_bbox_adapter_addr;
    u64 max_addr = g_bbox_adapter_addr + g_ap_module_info.log_len;
    if ((addr < min_addr) || (addr >= max_addr)) {
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

/*
 * @brief       : get data vaddr, excluding struct ap_root space
 * @return      : virtual add
 */
STATIC u64 bbox_adapter_get_data_vaddr(void)
{
    const u32 size = 10; // 1K alignment
    u32 timers = ((u32)sizeof(struct ap_root) >> size) + 1U;
    u64 vaddr = g_bbox_adapter_addr + ALIGN((u32)sizeof(struct ap_root), timers * size);
    if (bbox_adapter_check_vaddr(vaddr) != BBOX_SUCCESS) {
        BB_PRINT_ERR("there is no space left for ap to dump!\n");
        return 0;
    }
    return vaddr;
}

/*
 * @brief       : set reg value
 * @param [out] : regs_info *reg        reg
 * @param [in]  : const char *name      reg name
 * @param [in]  : u32 nlen              reg name length
 * @param [in]  : u64 addr              reg addr
 * @param [in]  : u32 size              reg space size
 * @return      : != 0 failure; ==0 success
 */
STATIC s32 bbox_adapter_set_reg(regs_info *reg, const char *name, u32 nlen, u64 addr, u32 size)
{
    s32 ret;
    u32 len;

    BB_CHECK_PTR(reg, return BBOX_FAILURE, "invalid param, reg is NULL.\n");
    BB_CHECK_PTR(name, return BBOX_FAILURE, "invalid param, name is NULL.\n");
    BB_CHECK_EXP_ACT(nlen == 0, return BBOX_FAILURE, "invalid param, nlen is zero.\n");
    BB_CHECK_EXP_ACT(addr == 0, return BBOX_FAILURE, "invalid param, addr is zero.\n");
    BB_CHECK_EXP_ACT(size == 0, return BBOX_FAILURE, "invalid param, ize is zero.\n");

    len = BBOX_MIN(nlen, (REG_NAME_LEN - 1U));
    ret = strncpy_s(reg->reg_name, REG_NAME_LEN, name, len);
    BB_CHECK_STRNCPY(ret, return BBOX_FAILURE);

    reg->reg_name[REG_NAME_LEN - 1U] = '\0';
    reg->reg_base = addr;
    reg->reg_size = size;
    reg->reg_map_addr = (u64)(uintptr_t)bbox_register_ioremap(reg->reg_name, (u32)strlen(reg->reg_name), addr, size);
    if (reg->reg_map_addr == 0) {
        BB_PRINT_ERR("unable to map [%s] registers\n", reg->reg_name);
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : free all mapped devices registers
 * @return      : NA
 */
STATIC void bbox_adapter_free_regs(void)
{
    s32 i;
    u32 devid;
    u32 max_devid = BBOX_MIN(g_bbox_ap_root->top_head.device_num, DEVICE_MAX_NUM);

    for (devid = 0; devid < max_devid; devid++) {
        regs_info *regs = g_bbox_ap_root->area_info.dump_regs_info[devid];
        for (i = 0; i < (s32)BBOX_MIN(g_bbox_ap_root->area_info.num_reg_regions, REGS_DUMP_MAX_NUM); i++) {
            bbox_register_iounmap(regs[i].reg_name, (u32)strnlen(regs[i].reg_name, REG_NAME_LEN),
                                  (void *)(uintptr_t)regs[i].reg_map_addr);
            regs[i].reg_map_addr = 0;
            regs[i].reg_dump_addr = 0;
        }
    }
}

/*
 * @brief       : dump registers for all devices
 * @return      : NA
 */
STATIC void bbox_adapter_regs_dump(void)
{
    u32 i, devid;
    u32 max_devid = BBOX_MIN(g_bbox_ap_root->top_head.device_num, DEVICE_MAX_NUM);
    enum reg_type_list type;

    for (devid = 0; devid < max_devid; devid++) {
        const regs_info *info = g_bbox_ap_root->area_info.dump_regs_info[devid];
        for (i = 0; i < BBOX_MIN(g_bbox_ap_root->area_info.num_reg_regions, REGS_DUMP_MAX_NUM); i++) {
            if ((info[i].reg_map_addr == 0) || (info[i].reg_dump_addr == 0) || (info[i].reg_size == 0)) {
                BB_PRINT_ERR("regs_info[%u]: reg_map_addr or reg_dump_addr is invalid!\n", i);
                continue;
            }

            type = bbox_register_type(info[i].reg_name, (u32)strnlen(info[i].reg_name, REG_NAME_LEN));
            bbox_register_dump(type,
                               (u8 *)(uintptr_t)info[i].reg_dump_addr, info[i].reg_size,
                               (u8 *)(uintptr_t)info[i].reg_map_addr, info[i].reg_size);
        }
    }
}

/*
 * @brief       : initialize & map all devices' registers add
 * @return      : <0 failure; ==0 success
 */
STATIC s32 bbox_adapter_reg_init(void)
{
    u32 max_devid = BBOX_MIN(g_bbox_ap_root->top_head.device_num, DEVICE_MAX_NUM);
    const struct bbox_config_data *config = bbox_get_config();
    u64 reg_dump_addr = bbox_adapter_get_data_vaddr();
    u32 max_reg_regions, devid, i;
    s32 ret;

    BB_CHECK_PTR(config, return BBOX_FAILURE, "get config failed.\n");
    BB_CHECK_EXP_ACT(reg_dump_addr == 0, return BBOX_FAILURE, "get data vaddr failed.\n");

    // init regs for all devices
#ifndef BBOX_UT
    max_reg_regions = BBOX_MIN(config->ap_regs_info.reg_num, REGS_DUMP_MAX_NUM);
    g_bbox_ap_root->area_info.num_reg_regions = max_reg_regions;
    for (devid = 0; devid < max_devid; devid++) {
        regs_info *regs = g_bbox_ap_root->area_info.dump_regs_info[devid];
        for (i = 0; i < max_reg_regions; i++) {
            if (config->ap_regs_info.reg_name[i] == NULL) {
                BB_PRINT_ERR("Reg name is NULL. (devid=%u; regid=%u)\n", devid, i);
                bbox_adapter_free_regs();
                return BBOX_FAILURE;
            }
            ret = bbox_adapter_set_reg(&regs[i],
                                       config->ap_regs_info.reg_name[i],
                                       (u32)strlen(config->ap_regs_info.reg_name[i]),
                                       bbox_register_get_device_addr(DEFAULT_DEVICE_ID, config->ap_regs_info.addr[i]),
                                       (u32)config->ap_regs_info.size[i]);
            if (ret != BBOX_SUCCESS) {
                BB_PRINT_ERR("failed to set reg %s!\n", config->ap_regs_info.reg_name[i]);
                bbox_adapter_free_regs();
                return BBOX_FAILURE;
            }
            regs[i].reg_dump_addr = reg_dump_addr;
            reg_dump_addr = reg_dump_addr + regs[i].reg_size;
        }
    }
#endif

    if (bbox_adapter_check_vaddr(reg_dump_addr) != BBOX_SUCCESS) {
        BB_PRINT_ERR("there is no space left for ap to dump regs!\n");
        bbox_adapter_free_regs();
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

static inline void bbox_adapter_callbock(bbox_dump_done_ops pfn_cb,
                                         const struct bbox_dump_done_ops_info *dump_done_info)
{
    if (pfn_cb != NULL) {
        pfn_cb(dump_done_info);
    }
}

/*
 * @brief       : initialize ap dump structure
 * @param [in]  : const struct bbox_dump_ops_info *info  dump info
 * @param [in]  : bbox_dump_done_ops pfn_cb    dump done
 * @return      : != 0 failure; ==0 success
 */
STATIC void bbox_adapter_dump(const struct bbox_dump_ops_info *info, bbox_dump_done_ops pfn_cb)
{
    struct bbox_dump_done_ops_info dump_done_info = {0};
    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");

    dump_done_info.devid = info->devid;
    dump_done_info.coreid = info->coreid;
    dump_done_info.excepid = info->excepid;
    dump_done_info.etype = info->etype;
    dump_done_info.time = info->time;

    if (!g_ap_init) {
        BB_PRINT_ERR("ap not initialized.\n");
        bbox_adapter_callbock(pfn_cb, &dump_done_info);
        return;
    }

    g_bbox_ap_root->current_info.devid = info->devid;
    g_bbox_ap_root->current_info.excepid = info->excepid;
    g_bbox_ap_root->current_info.etype = info->etype;
    g_bbox_ap_root->current_info.coreid = info->coreid;
    g_bbox_ap_root->current_info.tm.tv_sec = info->time.tv_sec;
    g_bbox_ap_root->current_info.tm.tv_nsec = info->time.tv_usec * KILO;

    if (info->excepid == (u32)EXCEPID_AP_PANIC) {
        bbox_adapter_save_panic_log(g_bbox_ap_root->current_info.devid,
                                    g_bbox_ap_root->current_info.excepid,
                                    g_bbox_ap_root->current_info.etype,
                                    g_bbox_ap_root->current_info.coreid,
                                    &g_bbox_ap_root->current_info.tm);
    } else {
        bbox_adapter_save_run_log(g_bbox_ap_root->current_info.devid,
                                  g_bbox_ap_root->current_info.excepid,
                                  g_bbox_ap_root->current_info.etype,
                                  g_bbox_ap_root->current_info.coreid,
                                  &g_bbox_ap_root->current_info.tm);
    }
    bbox_adapter_regs_dump();
    bbox_adapter_print_info();
    bbox_adapter_callbock(pfn_cb, &dump_done_info);
}

/*
 * @brief       : AP reset function when an exception occurs
 * @param [in]  : const struct bbox_reset_ops_info *info  reset info
 * @return      : NA
 */
STATIC void bbox_adapter_reset(const struct bbox_reset_ops_info *info)
{
    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");
    BB_PRINT_INFO("wait for reset. Exception info: devid[0x%x] coreid[0x%hhx] excepid[0x%x] etype[0x%hhx].\n",
                  info->devid, info->coreid, info->excepid, info->etype);

    bbox_adapter_flush_log_cache();
    printk_safe_flush_on_panic();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    console_flush_on_panic(CONSOLE_FLUSH_PENDING);
#else
    console_flush_on_panic();
#endif
    DEAD_WHILE;
}

/*
 * @brief       : init log info data
 * @return      : NA
 */
STATIC void bbox_adapter_loginfo_init(void)
{
    s32 ret;
    size_t size = sizeof(struct ap_log_info);
    BB_CHECK_PTR(g_bbox_ap_root, return, "invalid param, g_bbox_ap_root is NULL.\n");
    ret = memset_s(&g_bbox_ap_root->log_info, size, 0, size);
    BB_CHECK_MEMSET(ret, return);
}

/*
 * @brief       : initialize ap dump structure
 * @return      : != 0 failure; ==0 success
 */
STATIC s32 bbox_adapter_data_init(void)
{
    s32 ret = memset_s((void *)(uintptr_t)g_bbox_adapter_addr, g_ap_module_info.log_len, 0, g_ap_module_info.log_len);
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

    g_bbox_ap_root = (struct ap_root *)(uintptr_t)g_bbox_adapter_addr;

    // top
    g_bbox_ap_root->top_head.dump_magic = ADAPTER_DUMP_MAGIC;
    g_bbox_ap_root->top_head.version = ADAPTER_VERSION;
    g_bbox_ap_root->top_head.cpu_num = num_possible_cpus();
    g_bbox_ap_root->top_head.device_num = bbox_get_device_num();
    g_bbox_ap_root->top_head.end_magic = ADAPTER_DUMP_END_MAGIC;

    // currnet
    g_bbox_ap_root->current_info.excepid = 0;
    g_bbox_ap_root->current_info.coreid = (u8)BBOX_OS;
    g_bbox_ap_root->current_info.etype = (u8)BBOX_EXCEPTION_REASON_INVALID;
    g_bbox_ap_root->current_info.tm.tv_sec = 0;
    g_bbox_ap_root->current_info.tm.tv_nsec = 0;

    // log
    bbox_adapter_loginfo_init();

    // area
    g_bbox_ap_root->area_info.ap_area_map_addr = g_bbox_adapter_addr;
    g_bbox_ap_root->area_info.ap_area_addr = g_ap_module_info.log_addr;
    g_bbox_ap_root->area_info.ap_area_len = g_ap_module_info.log_len;
    ret = bbox_adapter_reg_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("[%s], Os adapter reg init failed!\n", __func__);
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

static inline void bbox_adapter_data_exit(void)
{
    if (g_bbox_ap_root != NULL) {
        bbox_adapter_free_regs();
        g_bbox_ap_root = NULL;
    }
}

/*
 * @brief       : register exception to rdr
 * @return      : NA
 */
STATIC void bbox_adapter_register_exception(void)
{
    u32 i, ret;
    for (i = 0; i < (sizeof(g_einfo) / sizeof(struct bbox_exception_info)); i++) {
        g_einfo[i].e_callback = NULL;
        ret = bbox_register_exception(&g_einfo[i]);
        if (ret == 0) {
            BB_PRINT_ERR("Os adapter register exception[0x%x-0x%x] failed with %u.\n",
                         g_einfo[i].e_excepid, g_einfo[i].e_excepid_end, ret);
        }
    }
}

/*
 * @brief       : unregiste exception from rdr
 * @return      : NA
 */
STATIC void bbox_adapter_unregister_exception(void)
{
    u32 i;
    for (i = 0; i < (sizeof(g_einfo) / sizeof(struct bbox_exception_info)); i++) {
        (void)bbox_unregister_exception(g_einfo[i].e_excepid);
    }
}

/*
 * @brief       : register reset and dump function to rdr
 * @return      : <0 failure; ==0 success
 */
STATIC s32 bbox_adapter_register_core(void)
{
    s32 ret;
    struct bbox_module_info s_soc_ops = {BBOX_OS, NULL, NULL};
    struct bbox_module_result retinfo = {0, 0};

    s_soc_ops.ops_dump = bbox_adapter_dump;
    s_soc_ops.ops_reset = bbox_adapter_reset;

    ret = bbox_register_module(&s_soc_ops, &retinfo);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("Os adapter register module failed with %d.\n", ret);
        return ret;
    }

    if ((retinfo.log_addr == 0) || (retinfo.log_len == 0)) {
        (void)bbox_unregister_module(s_soc_ops.coreid);
        BB_PRINT_ERR("os register core(0x%hhx) failed!", s_soc_ops.coreid);
        return BBOX_FAILURE;
    }
    g_ap_module_info.log_addr = retinfo.log_addr;
    g_ap_module_info.log_len = retinfo.log_len;

    return BBOX_SUCCESS;
}

/*
 * @brief       : unregister reset and dump function to rdr
 * @return      : <0 failure; ==0 success
 */
STATIC void bbox_adapter_unregister_core(void)
{
    (void)bbox_unregister_module(BBOX_OS);
    g_ap_module_info.log_addr = 0;
    g_ap_module_info.log_len = 0;
}

static void bbox_adapter_panic_dump(const struct bbox_time *tm)
{
    struct bbox_dump_ops_info info;
    info.coreid = BBOX_OS;
    info.devid = 0;
    info.etype = OS_PANIC;
    info.excepid = (u32)EXCEPID_AP_PANIC;
    info.time.tv_sec = tm->tv_sec;
    info.time.tv_usec = tm->tv_nsec / KILO;
    bbox_adapter_dump(&info, NULL);
}

static void bbox_adapter_panic_reset(void)
{
    struct bbox_reset_ops_info info;
    info.coreid = BBOX_OS;
    info.devid = 0;
    info.etype = OS_PANIC;
    info.excepid = (u32)EXCEPID_AP_PANIC;
    bbox_adapter_reset(&info);
}

STATIC s32 bbox_adapter_panic_notify(struct notifier_block *nb, bbox_event_t event, bbox_arg_t *args);

STATIC struct notifier_block g_ap_panic_block = {
    .notifier_call = bbox_adapter_panic_notify,
    .priority = INT_MIN,
};

/*
 * @brief       : panic notify function
 * @param [in]  : struct bbox_notifier_block *nb   notifier block struct, not used
 * @param [in]  : bbox_event_t* event              not used
 * @param [in]  : bbox_arg_t* args                 input argument ptr
 * @return      : <0 failure; ==0 success
 */
STATIC s32 bbox_adapter_panic_notify(struct notifier_block *nb, bbox_event_t event, bbox_arg_t *args)
{
    struct bbox_time tm = {0, 0};
    const struct bbox_exception_info *einfo = NULL;

    UNUSED(event);
    BB_PRINT_INFO("===> enter panic notify!\n");
    if (&g_ap_panic_block != nb) {
        BB_PRINT_ERR("panic notifier callback. wrong callback for panic chain!");
        return BBOX_FAILURE;
    }
    BB_PRINT_INFO("panic event: %lu!\n", event);

    einfo = bbox_adapter_get_einfo(OS_PANIC);

    preempt_disable();
    (void)bbox_snapshot_dump_panic(args);
    bbox_get_safe_systime(&tm);
    rdr_syserr_process_for_os(einfo, &tm);
    bbox_adapter_panic_dump(&tm);
    bbox_adapter_panic_reset();
    preempt_enable();
    return BBOX_SUCCESS;
}

/*
 * @brief       : die notify function
 * @param [in]  : struct bbox_notifier_block *nb   notifier block struct, not used
 * @param [in]  : bbox_event_t event               not used
 * @param [in]  : bbox_arg_t *args                 input argument ptr
 * @return      : <0 failure; ==0 success
 */
STATIC s32 bbox_adapter_die_notify(struct notifier_block *nb, bbox_event_t event, bbox_arg_t *args)
{
    BB_PRINT_INFO("===> enter die notify!\n");

    UNUSED(nb);
    UNUSED(event);
    (void)bbox_snapshot_dump_die(args);
    return BBOX_SUCCESS;
}

struct notifier_block g_ap_die_block = {
    .notifier_call = bbox_adapter_die_notify,
    .priority = INT_MIN,
};

/*
 * @brief       : communication status notify function. caller guarantees input not NULL
 * @param [in]  : struct bbox_notifier_block *nb   notifier block struct, not used
 * @param [in]  : bbox_event_t event               not used
 * @param [in]  : bbox_arg_t *args                 input argument ptr
 * @return      : ==0 success
 */
STATIC s32 bbox_adapter_comm_notify(struct bbox_notifier_block *nb, bbox_event_t event, bbox_arg_t *args)
{
    s32 ret;
    const struct bbox_callback_info *info = (const struct bbox_callback_info *)args;

    BB_CHECK_PTR(args, return BBOX_FAILURE, "invalid param, args is NULL.\n");
    UNUSED(nb);
    UNUSED(event);

    ret = bbox_channel_need_record_status(info);
    if (ret != BBOX_TRUE) {
        return BBOX_SUCCESS;
    }

    if (info->new_status == CHANNEL_STATUS_UNNORMAL) {
        const struct bbox_exception_info *einfo = bbox_adapter_get_einfo(OS_COMM);
        if (einfo != NULL) {
            struct bbox_time tm = {0, 0};
            bbox_get_systime(&tm);
            rdr_save_history_log_for_comm(DEFAULT_DEVICE_ID, einfo, &tm);
            bbox_adapter_save_comm_log(DEFAULT_DEVICE_ID, einfo->e_excepid,
                                       einfo->e_exce_type, einfo->e_from_core, &tm);
            (void)bbox_snapshot_dump_comm(einfo, &tm);
        }
        bbox_record_exce_type(OS_COMM);
        return BBOX_SUCCESS;
    }
    if (info->new_status == CHANNEL_STATUS_NORMAL) {
        bbox_record_exce_type(DEVICE_COLDBOOT);
        return BBOX_SUCCESS;
    }
    return BBOX_SUCCESS;
}

STATIC struct bbox_notifier_block g_ap_comm_block = {
    .s_list = {NULL, NULL},
    .notifier_call = bbox_adapter_comm_notify
};

/*
 * @brief       : init adapter operation
 * @return      : <0 failure; ==0 success
 */
STATIC void bbox_adapter_operation_init(void)
{
    (void)bbox_register_channel_notifier(&g_ap_comm_block);
    (void)atomic_notifier_chain_register(&panic_notifier_list, &g_ap_panic_block);
    (void)register_die_notifier(&g_ap_die_block);
}

/*
 * @brief       : exit final operation
 * @return      : NA
 */
STATIC void bbox_adapter_operation_exit(void)
{
    (void)bbox_unregister_channel_notifier(&g_ap_comm_block);
    (void)atomic_notifier_chain_unregister(&panic_notifier_list, &g_ap_panic_block);
    (void)unregister_die_notifier(&g_ap_die_block);
}

static s32 bbox_adapter_basic_init(void)
{
    // register module
    s32 ret = bbox_adapter_register_core();
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return BBOX_FAILURE, "os adapter register module failed with %d.\n", ret);

    // register exception
    bbox_adapter_register_exception();

    // map all device-0's physical add on init
    ret = bbox_adapter_map();
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return BBOX_FAILURE, "init adapter map failed with %d.\n", ret);

    // data init
    ret = bbox_adapter_data_init();
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return BBOX_FAILURE, "init adapter data failed with %d.\n", ret);

    return BBOX_SUCCESS;
}

static inline void bbox_adapter_basic_exit(void)
{
    bbox_adapter_unregister_core();
    bbox_adapter_unregister_exception();
    bbox_adapter_data_exit();
    bbox_adapter_unmap();
}

/*
 * @brief       : init function
 * @return      : <0 failure; ==0 success
 */
s32 bbox_adapter_init(void)
{
    s32 ret;

    if (g_ap_init == true) {
        return BBOX_SUCCESS;
    }

    ret = bbox_adapter_basic_init();
    if (ret != BBOX_SUCCESS) {
        bbox_adapter_basic_exit();
        return BBOX_FAILURE;
    }

    // init snapshot
    ret = bbox_snapshot_init();
    BB_CHECK_RET(ret != BBOX_SUCCESS, "init snapshot failed with %d.\n", ret);

    // init oom
    ret = bbox_oom_init();
    BB_CHECK_RET(ret != BBOX_SUCCESS, "init oom notifier failed with %d.\n", ret);

    // init operation
    bbox_adapter_operation_init();

    g_ap_init = true;
    BB_PRINT_INFO("os adapter initialized.\n");
    return BBOX_SUCCESS;
}

/*
 * @brief       : os adapter exit
 * @return      : NA
 */
void bbox_adapter_exit(void)
{
    if (g_ap_init == true) {
        bbox_adapter_basic_exit();
        bbox_adapter_operation_exit();
        bbox_snapshot_exit();
        bbox_oom_exit();
        g_ap_init = false;
    }
}

