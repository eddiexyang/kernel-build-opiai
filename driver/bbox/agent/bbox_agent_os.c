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
#ifndef BBOX_UT
#include <asm/cacheflush.h>
#include <linux/cpumask.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/printk.h>
#include <linux/securec.h>
#include <linux/version.h>

#include "config/bbox_config.h"
#include "register/bbox_register.h"
#include "rdr/rdr_field_core.h"
#include "common/bbox_print.h"
#include "../bbox_inner.h"
#include "bbox_agent_os.h"


STATIC struct ap_root *g_bbox_ap_root = NULL;
STATIC u64 g_bbox_agent_os_addr = 0;
STATIC struct bbox_module_result g_ap_module_info;
STATIC struct rdr_struct_s *g_pbb = NULL;

STATIC void *bbox_agent_map(u32 devid, phys_addr_t paddr, size_t size)
{
    s32 ret;
    u64 reserved_addr, reserved_size;

    BB_CHECK_EXP_ACT(size == 0, return NULL, "Invalid size.\n");

    ret = bbox_config_get_rdr(&reserved_addr, &reserved_size);
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return NULL, "Get rdr phymem addr&size failed.\n");
    if ((paddr < reserved_addr) ||
        ((paddr + size) > (reserved_addr + reserved_size))) {
        BB_PRINT_ERR("Error dev[%u] BBox memory. Map size = [0x%lx], bbox reserved size = [0x%llx]\n",
                     devid, size, reserved_size);
        return NULL;
    }

    return ioremap_wc((phys_addr_t)bbox_get_device_addr(devid, paddr), size);
}

STATIC void bbox_agent_unmap(const void *vaddr)
{
    if (vaddr != NULL) {
        iounmap((void __iomem *)vaddr);
    }
}


STATIC void bbox_agent_os_unmap(void)
{
    if (g_bbox_agent_os_addr != 0) {
        bbox_agent_unmap((void *)(uintptr_t)g_bbox_agent_os_addr);
        g_bbox_agent_os_addr = 0;
    }
}

STATIC s32 bbox_agent_os_map(void)
{
    g_bbox_agent_os_addr = (u64)(uintptr_t)bbox_agent_map(DEFAULT_DEVICE_ID,
        g_ap_module_info.log_addr, g_ap_module_info.log_len);
    if (g_bbox_agent_os_addr == 0) {
        BB_PRINT_ERR("g_bbox_agent_os_addr is invalid.\n");
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

STATIC void bbox_agent_os_save_log(u32 devid, u32 excepid, u8 etype,
    u8 coreid, const struct bbox_time *tm)
{
    s32 idx;
    BB_CHECK_PTR(tm, return, "Invalid param, time is NULL.\n");

    idx = g_bbox_ap_root->core_log_info.next_valid_index;
    if (idx < 0 || idx >= AP_LOG_BUFFER_NUM) {
        BB_PRINT_ERR("Invalid param, idx=%d.\n", idx);
        return;
    }

    g_bbox_ap_root->core_log_info.next_valid_index++;
    g_bbox_ap_root->core_log_info.next_valid_index %= AP_LOG_BUFFER_NUM;
    if (g_bbox_ap_root->core_log_info.log_num < AP_LOG_BUFFER_NUM) {
        g_bbox_ap_root->core_log_info.log_num++;
    }

    g_bbox_ap_root->core_log_info.log_buffer[idx].devid = devid;
    g_bbox_ap_root->core_log_info.log_buffer[idx].excepid = excepid;
    g_bbox_ap_root->core_log_info.log_buffer[idx].etype = etype;
    g_bbox_ap_root->core_log_info.log_buffer[idx].coreid = coreid;
    bbox_get_date(tm, g_bbox_ap_root->core_log_info.log_buffer[idx].date, DATATIME_MAXLEN);
}

static void bbox_agent_os_save_panic_log(u32 devid, u32 excepid, u8 etype, u8 coreid, const struct bbox_time *tm)
{
    bbox_agent_os_save_log(devid, excepid, etype, coreid, tm);
}

STATIC void bbox_agent_os_print_info(void)
{
    BB_PRINT_INFO("=================OS INFO================\n");
    BB_PRINT_INFO("version         [0x%x]\n", g_bbox_ap_root->core_top_head.version);
    BB_PRINT_INFO("devid           [0x%x]\n", g_bbox_ap_root->core_current_info.devid);
    BB_PRINT_INFO("coreid          [0x%hhx]\n", g_bbox_ap_root->core_current_info.coreid);
    BB_PRINT_INFO("excepid         [0x%x]\n", g_bbox_ap_root->core_current_info.excepid);
    BB_PRINT_INFO("etype           [0x%hhx]\n", g_bbox_ap_root->core_current_info.etype);
}

STATIC void bbox_agent_os_regs_dump(void)
{
    u32 i, devid;
    u32 max_devid = BBOX_MIN(g_bbox_ap_root->core_top_head.device_num, DEVICE_MAX_NUM);
    enum reg_type_list type;

    for (devid = 0; devid < max_devid; devid++) {
        const regs_info *info = g_bbox_ap_root->area_info.dump_regs_info_core[devid];
        for (i = 0; i < BBOX_MIN(g_bbox_ap_root->area_info.num_reg_regions_core, REGS_DUMP_MAX_NUM); i++) {
            if ((info[i].reg_map_addr == 0) || (info[i].reg_dump_addr == 0) || (info[i].reg_size == 0)) {
                BB_PRINT_ERR("Regs_info[%u]: reg_map_addr or reg_dump_addr is invalid!\n", i);
                continue;
            }

            type = bbox_register_type(info[i].reg_name, (u32)strnlen(info[i].reg_name, REG_NAME_LEN));
            bbox_register_dump(type,
                               (u8 *)(uintptr_t)info[i].reg_dump_addr, info[i].reg_size,
                               (u8 *)(uintptr_t)info[i].reg_map_addr, info[i].reg_size);
        }
    }
}

STATIC void bbox_agent_os_dump(const struct bbox_dump_ops_info *info, bbox_dump_done_ops pfn_cb)
{
    struct bbox_dump_done_ops_info dump_done_info = {0};
    BB_CHECK_PTR(info, return, "Invalid param, info is NULL.\n");

    dump_done_info.devid = info->devid;
    dump_done_info.coreid = info->coreid;
    dump_done_info.excepid = info->excepid;
    dump_done_info.etype = info->etype;
    dump_done_info.time = info->time;

    g_bbox_ap_root->core_current_info.devid = info->devid;
    g_bbox_ap_root->core_current_info.excepid = info->excepid;
    g_bbox_ap_root->core_current_info.etype = info->etype;
    g_bbox_ap_root->core_current_info.coreid = info->coreid;
    g_bbox_ap_root->core_current_info.tm.tv_sec = info->time.tv_sec;
    g_bbox_ap_root->core_current_info.tm.tv_nsec = info->time.tv_usec * KILO;

    bbox_agent_os_save_panic_log(g_bbox_ap_root->core_current_info.devid,
        g_bbox_ap_root->core_current_info.excepid, g_bbox_ap_root->core_current_info.etype,
        g_bbox_ap_root->core_current_info.coreid, &g_bbox_ap_root->core_current_info.tm);

    bbox_agent_os_regs_dump();
    bbox_agent_os_print_info();
}

STATIC s32 bbox_agent_os_check_vaddr(u64 addr)
{
    u64 min_addr = g_bbox_agent_os_addr;
    u64 max_addr = g_bbox_agent_os_addr + g_ap_module_info.log_len;
    if ((addr < min_addr) || (addr >= max_addr)) {
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

STATIC u64 bbox_agent_os_get_data_vaddr(void)
{
    const u32 size = 10;
    u32 timers = ((u32)sizeof(struct ap_root) >> size) + 1U;
    u64 vaddr = g_bbox_agent_os_addr + ALIGN((u32)sizeof(struct ap_root), timers * size);
    if (bbox_agent_os_check_vaddr(vaddr) != BBOX_SUCCESS) {
        BB_PRINT_ERR("There is no space left for ap to dump!\n");
        return 0;
    }
    return vaddr;
}

STATIC s32 bbox_agent_os_set_reg(regs_info *reg, const char *name, u32 nlen, u64 addr, u32 size)
{
    s32 ret;
    u32 len;

    BB_CHECK_PTR(reg, return BBOX_FAILURE, "Invalid param, reg is NULL.\n");
    BB_CHECK_PTR(name, return BBOX_FAILURE, "Invalid param, name is NULL.\n");
    BB_CHECK_EXP_ACT(nlen == 0, return BBOX_FAILURE, "Invalid param, nlen is zero.\n");
    BB_CHECK_EXP_ACT(addr == 0, return BBOX_FAILURE, "Invalid param, addr is zero.\n");
    BB_CHECK_EXP_ACT(size == 0, return BBOX_FAILURE, "Invalid param, ize is zero.\n");

    len = BBOX_MIN(nlen, (REG_NAME_LEN - 1U));
    ret = strncpy_s(reg->reg_name, REG_NAME_LEN, name, len);
    BB_CHECK_STRNCPY(ret, return BBOX_FAILURE);

    reg->reg_name[REG_NAME_LEN - 1U] = '\0';
    reg->reg_base = addr;
    reg->reg_size = size;
    reg->reg_map_addr = (u64)(uintptr_t)bbox_register_ioremap(reg->reg_name, (u32)strlen(reg->reg_name), addr, size);
    if (reg->reg_map_addr == 0) {
        BB_PRINT_ERR("Unable to map [%s] registers\n", reg->reg_name);
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

STATIC void bbox_agent_os_free_regs(void)
{
    s32 i;
    u32 devid;
    u32 max_devid = BBOX_MIN(g_bbox_ap_root->core_top_head.device_num, DEVICE_MAX_NUM);

    for (devid = 0; devid < max_devid; devid++) {
        regs_info *regs = g_bbox_ap_root->area_info.dump_regs_info_core[devid];
        for (i = 0; i < (s32)BBOX_MIN(g_bbox_ap_root->area_info.num_reg_regions_core, REGS_DUMP_MAX_NUM); i++) {
            bbox_register_iounmap(regs[i].reg_name, (u32)strnlen(regs[i].reg_name, REG_NAME_LEN),
                                  (void *)(uintptr_t)regs[i].reg_map_addr);
            regs[i].reg_map_addr = 0;
            regs[i].reg_dump_addr = 0;
        }
    }
}

STATIC s32 bbox_agent_os_reg_init(void)
{
    s32 i, ret;
    u32 devid;
    u32 max_devid = BBOX_MIN(g_bbox_ap_root->core_top_head.device_num, DEVICE_MAX_NUM);
    const struct bbox_config_data *config = bbox_get_config();
    u64 reg_dump_addr = bbox_agent_os_get_data_vaddr();

    BB_CHECK_PTR(config, return BBOX_FAILURE, "Get config failed.\n");
    BB_CHECK_EXP_ACT(reg_dump_addr == 0, return BBOX_FAILURE, "Get data vaddr failed.\n");

    g_bbox_ap_root->area_info.num_reg_regions_core = BBOX_MIN(config->ap_regs_info.reg_num, REGS_DUMP_MAX_NUM);
    for (devid = 0; devid < max_devid; devid++) {
        regs_info *regs = g_bbox_ap_root->area_info.dump_regs_info_core[devid];
        for (i = 0; i < (s32)g_bbox_ap_root->area_info.num_reg_regions_core; i++) {
            if (config->ap_regs_info.reg_name[i] == NULL) {
                BB_PRINT_ERR("Invalid reg name : NULL, devid : %u, regid : %d.\n", devid, i);
                bbox_agent_os_free_regs();
                return BBOX_FAILURE;
            }
            ret = bbox_agent_os_set_reg(&regs[i],
                                        config->ap_regs_info.reg_name[i],
                                        (u32)strlen(config->ap_regs_info.reg_name[i]),
                                        bbox_register_get_device_addr(devid, config->ap_regs_info.addr[i]),
                                        (u32)config->ap_regs_info.size[i]);
            if (ret != BBOX_SUCCESS) {
                BB_PRINT_ERR("Failed to set reg %s!\n", config->ap_regs_info.reg_name[i]);
                bbox_agent_os_free_regs();
                return BBOX_FAILURE;
            }
            regs[i].reg_dump_addr = reg_dump_addr;
            reg_dump_addr = reg_dump_addr + regs[i].reg_size;
        }
    }

    if (bbox_agent_os_check_vaddr(reg_dump_addr) != BBOX_SUCCESS) {
        BB_PRINT_ERR("There is no space left for ap to dump regs.(reg_dump_addr = %pK)\n", reg_dump_addr);
        bbox_agent_os_free_regs();
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

STATIC void bbox_agent_os_loginfo_init(void)
{
    s32 ret;
    size_t size = sizeof(struct ap_log_info);
    BB_CHECK_PTR(g_bbox_ap_root, return, "Invalid param, g_bbox_ap_root is NULL.\n");
    ret = memset_s(&g_bbox_ap_root->core_log_info, size, 0, size);
    BB_CHECK_MEMSET(ret, return);
}

STATIC s32 bbox_agent_os_data_init(void)
{
    s32 ret;

    g_bbox_ap_root = (struct ap_root *)(uintptr_t)g_bbox_agent_os_addr;

    // top
    g_bbox_ap_root->core_top_head.dump_magic = ADAPTER_DUMP_MAGIC;
    g_bbox_ap_root->core_top_head.version = ADAPTER_VERSION;
    g_bbox_ap_root->core_top_head.cpu_num = num_online_cpus();
    g_bbox_ap_root->core_top_head.device_num = bbox_get_device_num();
    g_bbox_ap_root->core_top_head.end_magic = ADAPTER_DUMP_END_MAGIC;

    // currnet
    g_bbox_ap_root->core_current_info.excepid = 0;
    g_bbox_ap_root->core_current_info.coreid = (u8)BBOX_AOS_CORE;
    g_bbox_ap_root->core_current_info.etype = (u8)BBOX_EXCEPTION_REASON_INVALID;
    g_bbox_ap_root->core_current_info.tm.tv_sec = 0;
    g_bbox_ap_root->core_current_info.tm.tv_nsec = 0;

    // log
    bbox_agent_os_loginfo_init();

    // area
    g_bbox_ap_root->area_info.ap_area_addr = g_ap_module_info.log_addr;
    g_bbox_ap_root->area_info.ap_area_len = g_ap_module_info.log_len;
    ret = bbox_agent_os_reg_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("[%s], Os adapter reg init failed!\n", __func__);
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

static inline void bbox_agent_os_data_exit(void)
{
    if (g_bbox_ap_root != NULL) {
        bbox_agent_os_free_regs();
        g_bbox_ap_root = NULL;
    }
}

static void bbox_agent_os_panic_dump(const struct bbox_time *tm)
{
    struct bbox_dump_ops_info info;
    info.coreid = BBOX_AOS_CORE;
    info.devid = 0;
    info.etype = OS_PANIC;
    info.excepid = (u32)EXCEPID_AP_PANIC;
    info.time.tv_sec = tm->tv_sec;
    info.time.tv_usec = tm->tv_nsec / KILO;
    bbox_agent_os_dump(&info, NULL);
}

STATIC void bbox_agent_os_save_history_log(const struct rdr_log_record *record)
{
    s32 ret;
    s32 idx;

    idx = g_pbb->core_log_info.next_valid_index;
    if (idx < 0 || idx >= RDR_LOG_BUFFER_NUM) {
        BB_PRINT_ERR("Invalid param, idx=%d.\n", idx);
        return;
    }

    g_pbb->core_log_info.next_valid_index++;
    g_pbb->core_log_info.next_valid_index %= RDR_LOG_BUFFER_NUM;
    if (g_pbb->core_log_info.log_num < RDR_LOG_BUFFER_NUM) {
        g_pbb->core_log_info.log_num++;
    }

    ret = memcpy_s(&(g_pbb->core_log_info.log_buffer[idx]), sizeof(struct rdr_log_record),
                   record, sizeof(struct rdr_log_record));
    BB_CHECK_MEMCPY(ret, return);
}

STATIC void bbox_agent_os_save_history_log_for_panic(const struct bbox_exception_info *info,
    const struct bbox_time *tm)
{
    struct rdr_log_record log_record;

    BB_CHECK_PTR(info, return, "Invalid param, info is NULL.\n");
    BB_CHECK_PTR(tm, return, "Invalid param, time is NULL.\n");

    log_record.record_type = RDR_RECORD_DEFINE_EXCEPTION;
    log_record.record.devid = 0;
    log_record.record.excepid = info->e_excepid;
    log_record.record.e_exce_type = info->e_exce_type;
    log_record.record.e_from_core = info->e_from_core;
    bbox_get_date(tm, log_record.record.date, DATATIME_MAXLEN);
    bbox_agent_os_save_history_log(&log_record);
}

STATIC void bbox_agent_os_syserr_proc_for_os(const struct bbox_exception_info *einfo,
    const struct bbox_time *tm)
{
    s32 ret;

    g_pbb->core_base_info.excepid = einfo->e_excepid;
    g_pbb->core_base_info.devid = 0;
    g_pbb->core_base_info.arg = 0;
    if (tm != NULL) {
        g_pbb->core_base_info.tm.tv_sec = tm->tv_sec;
        g_pbb->core_base_info.tm.tv_nsec = tm->tv_nsec;
        bbox_get_date(tm, g_pbb->core_base_info.date, DATATIME_MAXLEN);
    } else {
        g_pbb->core_base_info.tm.tv_sec = 0;
        g_pbb->core_base_info.tm.tv_nsec = 0;
        g_pbb->core_base_info.date[0] = '\0';
    }
    g_pbb->core_base_info.e_core = einfo->e_from_core;
    g_pbb->core_base_info.e_type = einfo->e_exce_type;
    ret = memcpy_s(g_pbb->core_base_info.e_module, BBOX_MODULE_NAME_LEN, einfo->e_from_module, BBOX_MODULE_NAME_LEN);
    BB_CHECK_MEMCPY(ret, return);
    ret = memcpy_s(g_pbb->core_base_info.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, einfo->e_desc, BBOX_EXCEPTIONDESC_MAXLEN);
    BB_CHECK_MEMCPY(ret, return);
    BB_PRINT_INFO("bbox_agent_os_syserr_proc_for_os: excepid=0x%x, e_core=%u, e_type=%u\n",
        g_pbb->core_base_info.excepid, g_pbb->core_base_info.e_core, g_pbb->core_base_info.e_type);

    bbox_agent_os_save_history_log_for_panic(einfo, tm);
    bbox_record_exce_type(einfo->e_exce_type);
}

STATIC struct module_core_map g_module_core[] = {
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
    {"TF",           BBOX_TF},
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
    {"DP",           BBOX_AOS_DP},
};

STATIC u32 get_module_core_map_size(void)
{
    return (u32)(sizeof(g_module_core) / sizeof(g_module_core[0]));
}

const char *bbox_agent_os_get_exception_core(u8 coreid)
{
    u32 i;
    for (i = 0; i < get_module_core_map_size(); i++) {
        if (coreid == g_module_core[i].coreid) {
            return g_module_core[i].name;
        }
    }

    return "UNDEF";
}

const char *bbox_agent_os_get_model_name(u32 excepid)
{
    u8 coreid = bbox_excepid_get_coreid(excepid);
    if (bbox_check_coreid_valid(coreid) == BBOX_SUCCESS) {
        return bbox_agent_os_get_exception_core(coreid);
    }

    return "UNDEF";
}

u8 bbox_agent_os_get_exception_coreid(const char *name)
{
    u32 i;
    for (i = 0; i < get_module_core_map_size(); i++) {
        if (strcmp(g_module_core[i].name, name) == 0) {
            return g_module_core[i].coreid;
        }
    }

    return "UNDEF";
}

STATIC s32 bbox_agent_os_area_init()
{
    s32 i;
    u64 sum_size = 0;
    struct rdr_area_mem_info info = {0};
    const struct bbox_config_data *config = bbox_get_config();

    BB_CHECK_PTR(config, return BBOX_FAILURE, "Invalid input param, config is NULL.\n");
    BB_PRINT_INFO("addr_start:%pK, addr_size:0x%x\n",
                  (void *)(uintptr_t)config->bbox_data.addr, config->bbox_data.length);

    info.end_addr = config->bbox_data.addr + config->bbox_data.length;
    info.num = BBOX_MIN(config->reserved_area.area_num, BBOX_AREA_MAXIMUM);
    for (i = 0; i < (s32)info.num; i++) {
        info.area_mem_id[i] = bbox_agent_os_get_exception_coreid(config->reserved_area.area_name[i]);
        info.area_mem_size[i] = config->reserved_area.area_size[i];
        sum_size += (u64)info.area_mem_size[i];
    }

    BB_CHECK_EXP_ACT((sum_size + RDR_BASEINFO_SIZE) > config->bbox_data.length,
        return BBOX_FAILURE, "Init rdr area size failed.\n");

    for (i = (s32)info.num - 1; i > 0; i--) {
        if (i == ((s32)info.num - 1)) {
            info.area_mem_addr[i] = info.end_addr - (u64)(info.area_mem_size[i]);
        } else {
            info.area_mem_addr[i] = info.area_mem_addr[i + 1] - (u64)(info.area_mem_size[i]);
        }
    }
    info.area_mem_addr[0] = config->bbox_data.addr + RDR_BASEINFO_SIZE;
    info.area_mem_size[0] = (u32)((info.area_mem_addr[1] - RDR_BASEINFO_SIZE) - config->bbox_data.addr);

    for (i = 0; i < (s32)info.num; i++) {
        g_pbb->area_info[i].coreid = info.area_mem_id[i];
        g_pbb->area_info[i].offset = info.area_mem_addr[i];
        g_pbb->area_info[i].length = info.area_mem_size[i];
    }
    g_pbb->top_head.area_number = info.num;

    return BBOX_SUCCESS;
}

s32 bbox_agent_get_areainfo(u8 coreid, u64 *addr, u32 *len)
{
    u32 i;

    for (i = 0; i < BBOX_AREA_MAXIMUM; i++) {
        if (coreid == g_pbb->area_info[i].coreid) {
            *len = g_pbb->area_info[i].length;
            *addr = g_pbb->area_info[i].offset;
            return BBOX_SUCCESS;
        }
    }
    *addr = 0;
    *len = 0;
    return BBOX_FAILURE;
}

STATIC s32 bbox_agent_os_get_osaddr()
{
    s32 ret;
    s32 i;
    u64 addr = 0;
    u64 size = 0;

    ret = bbox_config_get_rdr(&addr, &size);
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return BBOX_FAILURE, "Get rdr phymem addr&size failed.\n");

    g_pbb = (struct rdr_struct_s *)bbox_agent_map(0, addr, (size_t)size);
    BB_CHECK_PTR(g_pbb, return BBOX_FAILURE, "Map pbb failed.\n");

    g_pbb->top_head.magic = FILE_MAGIC;
    g_pbb->top_head.version = RDR_VERSION;
    ret = memcpy_s(g_pbb->top_head.product_name, RDR_PRODUCT_RELATION_LEN, "MDC", strlen("MDC"));
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);

    ret = bbox_agent_os_area_init();
    if (ret != BBOX_SUCCESS) {
        bbox_agent_unmap(g_pbb);
        return BBOX_FAILURE;
    }

    for (i = 0; i < (s32)BBOX_AREA_MAXIMUM; i++) {
        if (BBOX_OS == g_pbb->area_info[i].coreid) {
            g_ap_module_info.log_len = g_pbb->area_info[i].length;
            g_ap_module_info.log_addr = g_pbb->area_info[i].offset;
            return BBOX_SUCCESS;
        }
    }

    g_ap_module_info.log_len = 0;
    g_ap_module_info.log_addr = 0;
    return BBOX_SUCCESS;
}

s32 bbox_agent_os_panic_notify(struct notifier_block *nb, bbox_event_t event, bbox_arg_t *args);

STATIC struct notifier_block g_ap_panic_block = {
    .notifier_call = bbox_agent_os_panic_notify,
    .priority = INT_MIN,
};


s32 bbox_agent_os_panic_notify(struct notifier_block *nb, bbox_event_t event, bbox_arg_t *args)
{
    struct bbox_time tm = {0, 0};
    const struct bbox_exception_info einfo = {(u32)EXCEPID_AP_PANIC, (u32)EXCEPID_AP_PANIC,
        BBOX_COREID_MASK(BBOX_AOS_CORE), BBOX_COREID_MASK(BBOX_AOS_CORE),
        (u8)OS_PANIC, (u8)BBOX_AOS_CORE, (u8)BBOX_CRITICAL, (u8)BBOX_REBOOT_NOW,
        (u8)BBOX_REENTRANT_DISALLOW, "ap", "os panic", NULL
    };

    UNUSED(event);
    BB_PRINT_INFO("===> Enter aos-core panic notify.\n");

    if (&g_ap_panic_block != nb) {
        BB_PRINT_ERR("Panic notifier callback. wrong callback for panic chain.\n");
        return BBOX_FAILURE;
    }

    BB_PRINT_INFO("Panic event = %lu.\n", event);

    preempt_disable();
    bbox_get_safe_systime(&tm);
    bbox_agent_os_syserr_proc_for_os(&einfo, &tm);
    bbox_agent_os_panic_dump(&tm);
    preempt_enable();

    return BBOX_SUCCESS;
}

extern struct AtomicNotifierHead g_aosPanicNotifierList;

STATIC void bbox_agent_os_operation_init(void)
{
    (void)atomic_notifier_chain_register(&g_aosPanicNotifierList, &g_ap_panic_block);
}

STATIC void bbox_agent_os_operation_exit(void)
{
    (void)atomic_notifier_chain_unregister(&g_aosPanicNotifierList, &g_ap_panic_block);
}

static s32 bbox_agent_os_basic_init(void)
{
    s32 ret;

    ret = bbox_agent_os_get_osaddr();
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return BBOX_FAILURE, "Os adapter get addr failed. (ret=%d)\n", ret);

    ret = bbox_agent_os_map();
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return BBOX_FAILURE, "Init adapter map failed. (ret=%d)\n", ret);

    ret = bbox_agent_os_data_init();
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return BBOX_FAILURE, "Init adapter data failed. (ret=%d)\n", ret);

    return BBOX_SUCCESS;
}

static inline void bbox_agent_os_basic_exit(void)
{
    bbox_agent_os_data_exit();
    bbox_agent_os_unmap();
}

s32 bbox_agent_os_init(void)
{
    s32 ret;

    ret = bbox_agent_os_basic_init();
    if (ret != BBOX_SUCCESS) {
        bbox_agent_os_basic_exit();
        return BBOX_FAILURE;
    }

    bbox_agent_os_operation_init();

    BB_PRINT_INFO("Os adapter initialized.\n");
    return BBOX_SUCCESS;
}

void bbox_agent_os_exit(void)
{
    bbox_agent_os_basic_exit();
    bbox_agent_os_operation_exit();
}
#else
int bbox_agent_os_test(void)
{
    return 0;
}
#endif

