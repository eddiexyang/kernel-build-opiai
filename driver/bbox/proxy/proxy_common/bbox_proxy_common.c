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

#include "bbox_proxy_common.h"

#include <linux/securec.h>
#include <linux/delay.h>
#include "bbox_platform.h"
#include "bbox_proxy.h"
#include "bbox_proxy_config.h"
#include "bootparam/bbox_boot_param.h"
#include "communication/bbox_message.h"
#include "device/bbox_pub.h"
#include "device/bbox_pub_cloud.h"
#include "rdr/rdr_common.h"
#include "rdr/rdr_module_core.h"

#define MAX_DUMP_TIMEOUT  20000  // max dump timeout 20s

#define SET_DUMP_DONE_INFO(info, mgr) do { \
    (info).devid = (mgr)->current_info.devid; \
    (info).coreid = (mgr)->coreid; \
    (info).excepid = (mgr)->current_info.main_excepid; \
    (info).etype = (mgr)->current_info.etype; \
    (info).time = (mgr)->current_info.tm; \
} while (0)

#define SET_EXCEPTION_CTRL_INFO(e_ctrl, block_info) do { \
    (e_ctrl)->e_info_offset = (block_info).info_offset; \
    (e_ctrl)->e_info_len = (block_info).info_block_len; \
    (e_ctrl)->e_dump_status = STATUS_DONE; \
} while (0)

#define GET_TIMEOUT(timeout)      (((timeout) > MAX_DUMP_TIMEOUT) ? MAX_DUMP_TIMEOUT : (s32)(timeout))

#define IS_EXCEPID_MATCH(e_ctrl, mgr) \
    ((e_ctrl)->e_main_excepid == (mgr)->current_info.main_excepid)

#define IS_TMSTMP_MATCH(e_ctrl, mgr) \
    (((e_ctrl)->e_clock.tv_sec == (mgr)->current_info.tm.tv_sec) && \
     ((e_ctrl)->e_clock.tv_usec == (mgr)->current_info.tm.tv_usec))

STATIC struct bbox_proxy_mgr g_proxy_mgr;
STATIC s32 g_proxy_init = BBOX_FALSE;

static inline u32 bbox_proxy_normal_excepid_flag(u32 etype)
{
    return ((etype == (u32)HEARTBEAT_EXCEPTION) ? (u32)CHECK_HEARTBEAT_EXCEPID : (u32)CHECK_RUNTIME_EXCEPID);
}

static inline u32 bbox_proxy_normal_tmstmp_flag(u32 etype)
{
    return ((etype == (u32)HEARTBEAT_EXCEPTION) ? (u32)CHECK_HEARTBEAT_TMSTMP : (u32)CHECK_RUNTIME_TMSTMP);
}

static inline u32 bbox_proxy_current_excepid_flag(u32 etype)
{
    return ((etype == (u32)STARTUP_EXCEPTION) ? (u32)CHECK_STARTUP_EXCEPID : bbox_proxy_normal_excepid_flag(etype));
}

static inline u32 bbox_proxy_current_tmstmp_flag(u32 etype)
{
    return ((etype == (u32)STARTUP_EXCEPTION) ? (u32)CHECK_STARTUP_TMSTMP : bbox_proxy_normal_tmstmp_flag(etype));
}

static inline bool bbox_test_type_bit(u32 flag, u32 bit)
{
    return ((flag & bit) != 0);
}

static inline bool bbox_proxy_enable_dump_log(const struct bbox_proxy_module_mgr *mgr)
{
    return ((mgr->info->module.flag & (u64)BBOX_PROXY_CAPACITY_DUMP_LOG) != 0);
}

static inline bool bbox_proxy_enable_dump_ddr(const struct bbox_proxy_module_mgr *mgr)
{
    return ((mgr->info->module.flag & (u64)BBOX_PROXY_CAPACITY_DUMP_DDR) != 0);
}

static inline bool bbox_proxy_enable_trans_id(const struct bbox_proxy_module_mgr *mgr)
{
    return ((mgr->info->module.flag & (u64)BBOX_PROXY_CAPACITY_TRANS_ID) != 0);
}

static inline bool bbox_proxy_enable_register(const struct bbox_proxy_module_mgr *mgr)
{
    return ((mgr->info->module.flag & (u64)BBOX_PROXY_CAPACITY_REGISTER) != 0);
}

/*
 * @brief       : get proxy module manager
 * @param [in]  : u8 coreid             module id
 * @return      : proxy module manager
 */
STATIC struct bbox_proxy_module_mgr *bbox_proxy_get_module_mgr(u8 coreid)
{
    s32 i;
    for (i = 0; i < g_proxy_mgr.count; i++) {
        if (g_proxy_mgr.mgr[i].coreid == coreid) {
            return &g_proxy_mgr.mgr[i];
        }
    }
    return NULL;
}

/*
 * @brief       : dump proxy exception
 * @param [in]  : struct bbox_dump_ops_info *info           dump info
 * @param [in]  : bbox_dump_done_ops  fndone                dump done callback function
 * @return      : NA
 */
STATIC void bbox_proxy_dump(const struct bbox_dump_ops_info *info, bbox_dump_done_ops fndone)
{
    struct bbox_proxy_module_mgr *mgr = NULL;

    BB_CHECK_PTR(info, return, "invalid param, info is NULL.\n");
    BB_CHECK_PTR(fndone, return, "invalid param, fndone is NULL.\n");

    mgr = bbox_proxy_get_module_mgr(info->coreid);
    if (mgr == NULL) {
        BB_PRINT_ERR("bbox proxy dump, coreid : %d is not registered.\n", info->coreid);
        return;
    }
    mgr->current_info.devid = info->devid;
    mgr->current_info.main_excepid = info->excepid;
    mgr->current_info.sub_excepid = info->excepid;
    mgr->current_info.tm.tv_sec = info->time.tv_sec;
    mgr->current_info.tm.tv_usec = info->time.tv_usec;
    mgr->current_info.done = fndone;
    mgr->current_info.etype = info->etype;
    up(&mgr->sem);
}

/*
 * @brief       : check control block flag, and convert expid if matched & trans code defined
 * @param [in]  : u32 ctrl_flag                             block control flag
 * @param [in]  : u32 etype                                 reported excption type
 * @param [in]  : struct bbox_proxy_module_mgr *mgr          proxy module manager
 * @param [in]  : struct bbox_proxy_exception_ctrl *e_ctrl  control structure of proxy module
 * @return      : block matched: ==1; block not match ==0
 */
STATIC s32 bbox_proxy_match_config(u32 ctrl_flag, u32 etype,
    struct bbox_proxy_module_mgr *mgr, const struct bbox_proxy_exception_ctrl *e_ctrl)
{
    if (bbox_test_type_bit(ctrl_flag, bbox_proxy_current_excepid_flag(etype)) && !IS_EXCEPID_MATCH(e_ctrl, mgr)) {
        BB_PRINT_WARN("Excepid mismatch. (ctrl_flag=%u, etype=0x%x, e_ctrl_excepid=0x%x, mgr_excepid=0x%x)\n",
                      ctrl_flag, etype, e_ctrl->e_main_excepid, mgr->current_info.main_excepid);
        return BBOX_FALSE;
    }
    if (bbox_test_type_bit(ctrl_flag, bbox_proxy_current_tmstmp_flag(etype)) && !IS_TMSTMP_MATCH(e_ctrl, mgr)) {
        BB_PRINT_WARN("Tmstmp mismatch. (ctrl_flag=%u, etype=0x%x, e_ctrl_tv_sec=%llu, e_ctrl_tv_usec=%llu, "
                      "mgr_tv_sec=%llu, mgr_tv_usec=%llu)\n",
                      ctrl_flag, etype, e_ctrl->e_clock.tv_sec, e_ctrl->e_clock.tv_usec,
                      mgr->current_info.tm.tv_sec, mgr->current_info.tm.tv_usec);
        return BBOX_FALSE;
    }
    // transcode
    if (bbox_test_type_bit(ctrl_flag, bbox_proxy_current_excepid_flag(etype)) && bbox_proxy_enable_trans_id(mgr)) {
        mgr->current_info.sub_excepid = e_ctrl->e_sub_excepid;
    }
    return BBOX_TRUE;
}

/*
 * @brief       : get control structure of normal exception block match the current exception
 * @param [in]  : struct bbox_proxy_module_mgr *mgr          proxy module manager
 * @param [in]  : struct bbox_proxy_module_ctrl *m_ctrl     control structure of proxy module
 * @return      : control structure of exception block
 */
STATIC struct bbox_proxy_exception_ctrl *bbox_proxy_get_normal_exception_ctrl(struct bbox_proxy_module_mgr *mgr,
    struct bbox_proxy_module_ctrl *m_ctrl)
{
    s32 i;
    s32 e_block_num = (s32)BBOX_MIN(m_ctrl->config.e_block_num, BBOX_PROXY_CTRL_NUM);
    const s32 inter_ms = 500;
    s32 timeout = GET_TIMEOUT(mgr->info->module.wait_timeout);

    while (1) {
        s32 wait_flag = BBOX_FALSE;
        for (i = 0; i < e_block_num; i++) {
            struct bbox_proxy_exception_ctrl *e_ctrl = &m_ctrl->block[i];
            u32 etype = mgr->current_info.etype;
            if (!bbox_test_type_bit(m_ctrl->config.block_info[i].ctrl_type, (u16)BLOCK_TYPE_NORMAL)) {
                continue;
            }
            if (m_ctrl->block[i].e_dump_status != (u16)PROXY_STATUS_DONE) {
                BB_PRINT_INFO("Show block status. (block_id=%d, dump_status=%u)\n", i, m_ctrl->block[i].e_dump_status);
                wait_flag = BBOX_TRUE;
                continue;
            }
            if (bbox_proxy_match_config(m_ctrl->config.block_info[i].ctrl_flag, etype, mgr, e_ctrl) == BBOX_FALSE) {
                continue;
            }
            BB_PRINT_INFO("%s: find normal exception ctrl block id : %d.\n", mgr->thread_name, i);
            return e_ctrl;
        }
        if (wait_flag == BBOX_FALSE) {
            BB_PRINT_ERR("%s: cannot find normal exception ctrl.\n", mgr->thread_name);
            break;
        }
        if (timeout <= 0) {
            BB_PRINT_ERR("%s: wait block dump done timeout.\n", mgr->thread_name);
            break;
        }
        msleep(inter_ms);
        timeout -= inter_ms;
    }
    return NULL;
}

/*
 * @brief       : get control structure of startup exception block match the current exception
 * @param [in]  : struct bbox_proxy_module_mgr *mgr          proxy module manager
 * @param [in]  : struct bbox_proxy_module_ctrl *m_ctrl     control structure of proxy module
 * @return      : control structure of exception block
 */
STATIC struct bbox_proxy_exception_ctrl *bbox_proxy_get_startup_exception_ctrl(struct bbox_proxy_module_mgr *mgr,
                                                                               struct bbox_proxy_module_ctrl *m_ctrl)
{
    s32 i;
    s32 e_block_num = (s32)BBOX_MIN(m_ctrl->config.e_block_num, BBOX_PROXY_CTRL_NUM);
    BB_PRINT_INFO("%s: bbox get startup exception ctrl start.\n", mgr->thread_name);
    for (i = 0; i < e_block_num; i++) {
        struct bbox_proxy_exception_ctrl *e_ctrl = &m_ctrl->block[i];
        if (!bbox_test_type_bit(m_ctrl->config.block_info[i].ctrl_type, (u16)BLOCK_TYPE_STARTUP)) {
            continue;
        }
        if (bbox_test_type_bit(m_ctrl->config.block_info[i].ctrl_type, (u16)BLOCK_TYPE_NORMAL)) {
            return bbox_proxy_get_normal_exception_ctrl(mgr, m_ctrl);
        }
        if (bbox_proxy_match_config(m_ctrl->config.block_info[i].ctrl_flag,
                                    mgr->current_info.etype, mgr, e_ctrl) == BBOX_FALSE) {
            continue;
        }
        SET_EXCEPTION_CTRL_INFO(e_ctrl, m_ctrl->config.block_info[i]);
        BB_PRINT_INFO("%s: find startup exception ctrl block id : %d.\n", mgr->thread_name, i);
        return e_ctrl;
    }
    BB_PRINT_ERR("%s: bbox get startup exception ctrl failed.\n", mgr->thread_name);
    return NULL;
}

/*
 * @brief       : get control structure of exception block match the current exception
 * @param [in]  : struct bbox_proxy_module_mgr *mgr          proxy module manager
 * @param [in]  : struct bbox_proxy_module_ctrl *m_ctrl     control structure of proxy module
 * @return      : control structure of exception block
 */
STATIC struct bbox_proxy_exception_ctrl *bbox_proxy_get_exception_ctrl(struct bbox_proxy_module_mgr *mgr,
                                                                       struct bbox_proxy_module_ctrl *m_ctrl)
{
    if (mgr->current_info.etype == (u8)STARTUP_EXCEPTION) {
        return bbox_proxy_get_startup_exception_ctrl(mgr, m_ctrl);
    } else {
        return bbox_proxy_get_normal_exception_ctrl(mgr, m_ctrl);
    }
}

/*
 * @brief       : dump proxy ddr data
 * @param [in]  : struct bbox_proxy_module_mgr *mgr          proxy module manager
 * @param [in]  : struct bbox_proxy_exception_ctrl *e_ctrl  control structure of exception block
 * @param [in]  : u64 ddr_addr                              ddr add
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_proxy_dump_ddr(const struct bbox_proxy_module_mgr *mgr, const struct bbox_proxy_exception_ctrl *e_ctrl,
                               u64 ddr_addr)
{
    struct bbox_dump_done_ops_info info;
    u32 len;
    u64 addr;

    BB_PRINT_INFO(" =========bbox_proxy_dump_ddr========\n");
    BB_PRINT_INFO(" coreid:          [0x%hhx]\n", mgr->coreid);
    BB_PRINT_INFO(" e_info_offset:   [0x%x]\n", e_ctrl->e_info_offset);
    BB_PRINT_INFO(" e_info_len:      [0x%x]\n", e_ctrl->e_info_len);
    BB_PRINT_INFO(" excepid:         [0x%x]\n", mgr->current_info.main_excepid);
    BB_PRINT_INFO(" sub excepid:     [0x%x]\n", mgr->current_info.sub_excepid);
    BB_PRINT_INFO(" time:            [%llu.%llu]\n", mgr->current_info.tm.tv_sec, mgr->current_info.tm.tv_usec);
    BB_PRINT_INFO(" ====================================\n");

    if ((e_ctrl->e_info_offset >= sizeof(struct bbox_proxy_module_ctrl)) &&
        ((e_ctrl->e_info_offset + e_ctrl->e_info_len) <= mgr->ddr_len)) {
        addr = ddr_addr + e_ctrl->e_info_offset;
        len = e_ctrl->e_info_len;
        info.coreid = mgr->coreid;
        info.etype = mgr->current_info.etype;
        info.devid = mgr->current_info.devid;
        info.excepid = mgr->current_info.main_excepid;
        info.time = mgr->current_info.tm;
        return rdr_module_ddr_dump(&info, mgr->current_info.sub_excepid, (const char *)(uintptr_t)addr, len);
    } else {
        BB_PRINT_ERR("%s: invalid exception info, offset : 0x%x, len : 0x%x ",
                     mgr->thread_name, e_ctrl->e_info_offset, e_ctrl->e_info_len);
        return BBOX_FAILURE;
    }
}

/*
 * @brief       : dump proxy log data
 * @param [in]  : struct bbox_proxy_module_mgr *mgr    proxy module manager
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_proxy_dump_log(const struct bbox_proxy_module_mgr *mgr)
{
    struct bbox_dump_done_ops_info info;
    const char *log_addr = (const char *)bbox_ioremap(bbox_get_device_addr(mgr->current_info.devid,
        (phys_addr_t)mgr->info->module.log_addr), mgr->info->module.log_len);
    s32 ret;
    BB_CHECK_PTR(log_addr, return BBOX_FAILURE, "%s: map log_addr address failed, device-%u.\n",
                 mgr->thread_name, mgr->current_info.devid);
    info.coreid = mgr->coreid;
    info.etype = mgr->current_info.etype;
    info.devid = mgr->current_info.devid;
    info.excepid = mgr->current_info.main_excepid;
    info.time = mgr->current_info.tm;
    ret = rdr_module_log_dump(&info, mgr->current_info.sub_excepid, log_addr, mgr->info->module.log_len);
    bbox_iounmap((const void *)log_addr);
    log_addr = NULL;
    return ret;
}

/*
 * @brief       : send remote module data to hdc
 * @param [in]  : struct bbox_dump_done_ops_info *info       dump done info
 * @param [in]  : bbox_dump_done_ops fndone                  dump done function
 * @return      : NA
 */
void bbox_proxy_module_dump(const struct bbox_dump_done_ops_info *info, bbox_dump_done_ops fndone)
{
    s32 ret;
    u32 len;
    u32 log_len;
    u64 addr;
    u64 log_vaddr = 0;
    const struct exc_module_info_s *r_info = NULL;

    BB_CHECK_PTR(info, return, "invalid parameter. info : NULL.\n");

    ret = rdr_module_get_vaddr(info->devid, info->coreid, &log_vaddr);
    if ((ret <= 0) || (log_vaddr == 0)) {
        BB_PRINT_ERR("get module[%d] info failed with %d!!!\n", info->coreid, ret);
        return;
    }

    log_len = (u32)ret;
    r_info = (struct exc_module_info_s *)(uintptr_t)log_vaddr;
    if ((r_info->magic == MODULE_MAGIC) && (r_info->e_excep_valid == MODULE_VALID)) {
        if (((u64)r_info->e_info_offset + (u64)r_info->e_info_len) <= (u64)log_len) {
            addr = log_vaddr + r_info->e_info_offset;
            len = r_info->e_info_len;
            ret = rdr_module_ddr_dump(info, info->excepid, (const char *)(uintptr_t)addr, len);
            BB_CHECK_RET(ret != BBOX_SUCCESS, "bbox proxy dump ddr failed, coreid[0x%hhx], devid[%u], excepid[0x%x].\n",
                         info->coreid, info->devid, info->excepid);
        } else {
            BB_PRINT_ERR("param invalid! coreid[0x%hhx][0x%x], offset[0x%x], len[0x%x].",
                         info->coreid, log_len, r_info->e_info_offset, r_info->e_info_len);
        }
    } else {
        ret = rdr_module_ddr_dump(info, info->excepid, (const char *)(uintptr_t)log_vaddr, log_len);
        BB_CHECK_RET(ret != BBOX_SUCCESS, "bbox proxy dump ddr failed, coreid[0x%hhx], devid[%u], excepid[0x%x].\n",
                     info->coreid, info->devid, info->excepid);
    }

    rdr_module_free_vaddr(log_vaddr);
    log_vaddr = 0;
    if (fndone != NULL) {
        fndone(info);
    }
    return;
}

/*
 * @brief       : process proxy ddr data
 * @param [in]  : struct bbox_proxy_module_mgr *mgr    proxy module manager
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_proxy_ddr_dump_process(struct bbox_proxy_module_mgr *mgr)
{
    s32 ret;
    struct bbox_proxy_module_ctrl *m_ctrl = NULL;
    struct bbox_proxy_exception_ctrl *e_ctrl = NULL;

    m_ctrl = (struct bbox_proxy_module_ctrl *)rdr_map(mgr->current_info.devid,
                                                      (phys_addr_t)mgr->ddr_addr, mgr->ddr_len);
    BB_CHECK_PTR(m_ctrl, return BBOX_FAILURE, "%s: map address failed, device-%u.\n",
                 mgr->thread_name, mgr->current_info.devid);

    if ((m_ctrl->magic != BBOX_PROXY_MAGIC) || (m_ctrl->config.e_block_num == 0) ||
        (m_ctrl->config.e_block_num > BBOX_PROXY_CTRL_NUM)) {
        BB_PRINT_ERR("%s: ctrl data error, magic[0x%x], block num[%hhu].\n",
                     mgr->thread_name, m_ctrl->magic, m_ctrl->config.e_block_num);
        rdr_unmap((void *)m_ctrl);
        return BBOX_FAILURE;
    }

    // match block
    e_ctrl = bbox_proxy_get_exception_ctrl(mgr, m_ctrl);
    if (e_ctrl == NULL) {
        BB_PRINT_ERR("%s: cannot find exception ctrl for exception type 0x%hhx.\n",
                     mgr->thread_name, mgr->current_info.etype);
        rdr_unmap((void *)m_ctrl);
        return BBOX_FAILURE;
    }

    // dump ddr
    e_ctrl->e_save_status = PROXY_STATUS_DOING;
    ret = bbox_proxy_dump_ddr(mgr, e_ctrl, (u64)(uintptr_t)m_ctrl);
    e_ctrl->e_save_status = PROXY_STATUS_INIT;
    e_ctrl->e_dump_status = PROXY_STATUS_INIT;
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("%s: bbox proxy dump ddr failed whit %d.\n", mgr->thread_name, ret);
        rdr_unmap((void *)m_ctrl);
        return BBOX_FAILURE;
    }

    rdr_unmap((void *)m_ctrl);
    m_ctrl = NULL;
    return BBOX_SUCCESS;
}

/*
 * @brief       : dump proxy exception data
 * @param [in]  : struct bbox_proxy_module_mgr *mgr    proxy module manager
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_proxy_dump_exception(struct bbox_proxy_module_mgr *mgr)
{
    s32 ret;
    s32 result = BBOX_SUCCESS;
    struct bbox_dump_done_ops_info dump_done_info = {0};

    // dump ddr
    if (bbox_proxy_enable_dump_ddr(mgr)) {
        ret = bbox_proxy_ddr_dump_process(mgr);
        if (ret != BBOX_SUCCESS) {
            result = BBOX_FAILURE;
            BB_PRINT_ERR("%s : bbox proxy dump ddr failed : %d.\n", mgr->thread_name, ret);
        }
    }

    // dump done
    if (mgr->current_info.done != NULL) {
        SET_DUMP_DONE_INFO(dump_done_info, mgr);
        mgr->current_info.done(&dump_done_info);
    }

    // dump log
    if (bbox_proxy_enable_dump_log(mgr)) {
        ret = bbox_proxy_dump_log(mgr);
        if (ret != BBOX_SUCCESS) {
            result = BBOX_FAILURE;
            BB_PRINT_ERR("%s : bbox proxy dump log failed : %d.\n", mgr->thread_name, ret);
        }
    }
    return result;
}

/*
 * @brief       : init proxy module control structure
 * @param [in]  : const struct bbox_proxy_module_mgr *mgr    proxy module manager
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_proxy_module_ctrl_init(const struct bbox_proxy_module_mgr *mgr)
{
    s32 i;
    u64 log_vaddr;
    struct bbox_proxy_module_ctrl *m_ctrl = NULL;

    for (i = 0; i < (s32)bbox_get_device_num(); i++) {
        u32 len;
        s32 ret = rdr_module_get_vaddr((u32)i, mgr->coreid, &log_vaddr);
        if ((ret <= 0) || (log_vaddr == 0)) {
            BB_PRINT_ERR("get module[%d] info failed with %d!!!\n", mgr->coreid, ret);
            rdr_module_free_vaddr(log_vaddr);
            return BBOX_FAILURE;
        }

        m_ctrl = (struct bbox_proxy_module_ctrl *)(uintptr_t)log_vaddr;
        m_ctrl->config = mgr->info->ctrl;

        len = (u32)sizeof(struct bbox_proxy_exception_ctrl) * BBOX_PROXY_CTRL_NUM;
        ret = memset_s(m_ctrl->block, len, 0, len);
        if (ret != 0) {
            BB_PRINT_ERR("memset_s failed, devid : %d, coreid : %d.\n", i, mgr->coreid);
            rdr_module_free_vaddr(log_vaddr);
            return BBOX_FAILURE;
        }
        m_ctrl->magic = BBOX_PROXY_MAGIC;
        rdr_module_free_vaddr(log_vaddr);
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : register proxy module
 * @param [in]  : struct bbox_proxy_module_mgr *mgr    proxy module manager
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_proxy_register_module(struct bbox_proxy_module_mgr *mgr)
{
    s32 ret;
    struct bbox_module_result info = {0, 0};
    struct bbox_module_info s_module_ops;

    s_module_ops.coreid = mgr->info->module.coreid;
    s_module_ops.ops_dump = bbox_proxy_dump;
    s_module_ops.ops_reset = NULL;
    ret = bbox_register_module(&s_module_ops, &info);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("%s : register module failed! return %d\n", mgr->thread_name, ret);
        return BBOX_FAILURE;
    }

    // check ddr info
    if ((info.log_addr == 0) || (info.log_len == 0)) {
        BB_PRINT_ERR("%s : get module addr failed!", mgr->thread_name);
        (void)bbox_unregister_module(mgr->info->module.coreid);
        return BBOX_FAILURE;
    }

    // addr set
    mgr->ddr_len = info.log_len;
    mgr->ddr_addr = info.log_addr;

    // 拷贝bbox_proxy_ctrl_info结构体到共享内存
    ret = bbox_proxy_module_ctrl_init(mgr);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("%s : copy ctrl info to ddr failed! return %d\n", mgr->thread_name, ret);
        (void)bbox_unregister_module(mgr->info->module.coreid);
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}


/*
 * @brief       : unregister proxy module
 * @param [in]  : struct bbox_proxy_module_mgr *mgr    proxy module manager
 * @return      : NA
 */
STATIC void bbox_proxy_unregister_module(struct bbox_proxy_module_mgr *mgr)
{
    s32 ret = bbox_unregister_module(mgr->info->module.coreid);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("%s : unregister module failed! return %d\n", mgr->thread_name, ret);
        return;
    }
    mgr->ddr_len = 0;
    mgr->ddr_addr = 0;
}

/*
 * @brief       : register proxy exception
 * @param [in]  : struct bbox_proxy_module_mgr *mgr       proxy module manager
 * @param [in]  : struct bbox_proxy_exception_info *e    exception info
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_proxy_register_exception(const struct bbox_proxy_module_mgr *mgr,
                                         const struct bbox_proxy_exception_info *e)
{
    s32 ret;
    u32 excepid_end;
    struct bbox_exception_info einfo;

    ret = memset_s(&einfo, sizeof(einfo), 0, sizeof(einfo));
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

    einfo.e_excepid = e->e_excepid;
    einfo.e_excepid_end = e->e_excepid;
    einfo.e_process_priority = e->e_process_priority;
    einfo.e_reboot_priority = e->e_reboot_priority;
    einfo.e_notify_core_mask = e->e_notify_core_mask;
    einfo.e_reset_core_mask = e->e_reset_core_mask;
    einfo.e_from_core = mgr->info->module.coreid;
    einfo.e_reentrant = e->e_reentrant;
    einfo.e_exce_type = e->e_excep_type;
    ret = memcpy_s(einfo.e_from_module, BBOX_MODULE_NAME_LEN, mgr->info->module.name, strlen(mgr->info->module.name));
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
    ret = memcpy_s(einfo.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, e->e_desc, BBOX_EXCEPTIONDESC_MAXLEN);
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
    excepid_end = bbox_register_exception(&einfo);
    if (excepid_end == 0U) {
        BB_PRINT_ERR("%s : register exception failed with result[%u]. excepid[0x%x].\n",
            mgr->thread_name, excepid_end, einfo.e_excepid);
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}


/*
 * @brief       : register proxy exceptions
 * @param [in]  : const struct bbox_proxy_module_mgr *mgr    proxy module manager
 * @return      : <0 failure; =0 success
 */
STATIC void bbox_proxy_register_exceptions(const struct bbox_proxy_module_mgr *mgr)
{
    s32 i;
    u32 e_count = BBOX_MIN(mgr->info->module.e_count, BBOX_PROXY_EXCEPTION_NUM);

    for (i = 0; i < (s32)e_count; i++) {
        const struct bbox_proxy_exception_info *e = &mgr->info->exception[i];
        if (bbox_proxy_register_exception(mgr, e) != BBOX_SUCCESS) {
            BB_PRINT_ERR("%s : register exception [%u] failed.\n", mgr->thread_name, e->e_excepid);
        }
    }

    return;
}

/*
 * @brief       : unregister proxy exceptions
 * @param [in]  : struct bbox_proxy_module_mgr *mgr    proxy module manager
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_proxy_unregister_exceptions(const struct bbox_proxy_module_mgr *mgr)
{
    s32 i;
    s32 result = BBOX_SUCCESS;
    u32 e_count = BBOX_MIN(mgr->info->module.e_count, BBOX_PROXY_EXCEPTION_NUM);

    for (i = 0; i < (s32)e_count; i++) {
        const struct bbox_proxy_exception_info *e = &mgr->info->exception[i];
        s32 ret = bbox_unregister_exception(e->e_excepid);
        if (ret != BBOX_SUCCESS) {
            BB_PRINT_ERR("%s: unregister exception failed with ret[%d]. excepid[0x%x].\n",
                         mgr->thread_name, ret, e->e_excepid);
            result = BBOX_FAILURE;
        }
    }

    return result;
}

/*
 * @brief       : register proxy
 * @param [in]  : struct bbox_proxy_module_mgr *mgr    proxy module manager
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_proxy_register(struct bbox_proxy_module_mgr *mgr)
{
    // register module
    s32 ret = bbox_proxy_register_module(mgr);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("%s : bbox proxy register modules failed.\n", mgr->thread_name);
        return BBOX_FAILURE;
    }

    // register exception
    bbox_proxy_register_exceptions(mgr);
    return BBOX_SUCCESS;
}

/*
 * @brief       : unregister proxy
 * @param [in]  : struct bbox_proxy_module_mgr *mgr    proxy module manager
 * @return      : NA
 */
STATIC void bbox_proxy_unregister(struct bbox_proxy_module_mgr *mgr)
{
    // unregister exception
    s32 ret = bbox_proxy_unregister_exceptions(mgr);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("%s : bbox proxy unregister exception failed.\n", mgr->thread_name);
        return;
    }

    // unregister module
    bbox_proxy_unregister_module(mgr);
}

/*
 * @brief       : proxy thread
 * @param [in]  : void *arg         thread arg
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_proxy_thread(void *arg)
{
    s32 ret = BBOX_SUCCESS;
    struct bbox_proxy_module_mgr *mgr = NULL;

    BB_CHECK_PTR(arg, return BBOX_FAILURE, "invalid param, arg[NULL].\n");

    mgr = (struct bbox_proxy_module_mgr *)arg;
    BB_CHECK_PTR(mgr->info, return BBOX_FAILURE, "%s: invalid param, info[NULL].\n", mgr->thread_name);

    BB_PRINT_INFO("enter %s thread.\n", mgr->thread_name);
    if (bbox_proxy_enable_register(mgr)) {
        ret = bbox_proxy_register(mgr);
        if (ret != BBOX_SUCCESS) {
            return ret;
        }
    }

    // dump exception
    if (bbox_proxy_enable_dump_ddr(mgr) || bbox_proxy_enable_dump_log(mgr)) {
        while (!kthread_should_stop()) {
            bbox_jiffies_t rdr_jiffies = msecs_to_jiffies(WAIT_MS_LONG);
            if (down_timeout(&mgr->sem, (bbox_long_t)rdr_jiffies) != 0) {
                continue;
            }

            ret = bbox_proxy_dump_exception(mgr);
            BB_CHECK_RET(ret != BBOX_SUCCESS, "%s: dump process failed", mgr->thread_name);
        }
    }
    if (bbox_proxy_enable_register(mgr)) {
        bbox_proxy_unregister(mgr);
    }
    BB_PRINT_INFO("exit %s thread.\n", mgr->thread_name);
    mgr->thread = NULL;
    return ret;
}

/*
 * @brief       : init proxy module
 * @param [in]  : struct bbox_proxy_module_mgr *mgr    proxy module manager
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_proxy_module_init(struct bbox_proxy_module_mgr *mgr)
{
    s32 ret;
    const char *name = NULL;
    BB_CHECK_PTR(mgr, return BBOX_FAILURE, "invalid param, mgr is NULL.\n");
    BB_CHECK_PTR(mgr->info, return BBOX_FAILURE, "invalid param, mgr->info is NULL.\n");
    BB_CHECK_PTR(mgr->info->module.name, return BBOX_FAILURE, "invalid param, mgr->info->module.name is NULL.\n");

    if (mgr->initialized == BBOX_TRUE) {
        BB_PRINT_INFO("bbox proxy module %s has initialized.", mgr->info->module.name);
        return BBOX_SUCCESS;
    }

    // 启线程
    name = mgr->info->module.name;
    mgr->coreid = mgr->info->module.coreid;
    sema_init(&mgr->sem, 0);
    ret = sprintf_s(mgr->thread_name, THREAD_NAME_LEN, "%s_proxy", name);
    BB_CHECK_SPRINTF(ret, return BBOX_FAILURE);

    mgr->thread = bbox_kthread_proc(bbox_proxy_thread, mgr, (const char *)mgr->thread_name);
    if (mgr->thread == NULL) {
        BB_PRINT_ERR("%s: create thread bbox_proxy_thread failed.\n", name);
        ret = memset_s(mgr->thread_name, THREAD_NAME_LEN, 0, THREAD_NAME_LEN);
        BB_CHECK_SECUREC(BB_CHECK_MEMSET, ret);
        return BBOX_FAILURE;
    }

    mgr->initialized = BBOX_TRUE;
    BB_PRINT_INFO("bbox proxy module %s init success.", name);
    return BBOX_SUCCESS;
}

/*
 * @brief       : init proxy
 * @return      : <0 failure; =0 success
 */
s32 bbox_proxy_init(void)
{
    s32 i, ret, count;

    if (g_proxy_init == BBOX_TRUE) {
        BB_PRINT_INFO("proxy common is already initialized");
        return BBOX_SUCCESS;
    }
    ret = memset_s(&g_proxy_mgr, sizeof(struct bbox_proxy_mgr), 0, sizeof(struct bbox_proxy_mgr));
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

    ret = bbox_product_proxy_init(&g_proxy_mgr);
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return BBOX_FAILURE, "bbox product proxy init failed.\n");

    count = BBOX_MIN(g_proxy_mgr.count, (s32)BBOX_CORE_MAX);
    for (i = 0; i < count; i++) {
        ret = bbox_proxy_module_init(&g_proxy_mgr.mgr[i]);
        if (ret != BBOX_SUCCESS) {
            g_proxy_mgr.mgr[i].info = NULL;
            BB_PRINT_ERR("bbox proxy module %d init failed.", i);
        }
    }
    g_proxy_init = BBOX_TRUE;
    return BBOX_SUCCESS;
}

/*
 * @brief       : exit proxy
 * @return      : NA
 */
void bbox_proxy_exit(void)
{
    s32 i, ret, count;

    if (g_proxy_init != BBOX_TRUE) {
        return;
    }

    BB_PRINT_INFO("bbox proxy start exit.\n");
    count = BBOX_MIN(g_proxy_mgr.count, (s32)BBOX_CORE_MAX);
    for (i = 0; i < count; i++) {
        struct bbox_proxy_module_mgr *mgr = &g_proxy_mgr.mgr[i];
        if (mgr->initialized == BBOX_FALSE) {
            continue;
        }
        KTHREAD_STOP(mgr->thread);
        BB_PRINT_INFO("kthread %s stopped.\n", mgr->thread_name);
        mgr->info = NULL;
        mgr->ddr_addr = 0;
        mgr->ddr_len = 0;
        mgr->coreid = 0;
        sema_init(&mgr->sem, 0);

        mgr->current_info.devid = 0;
        mgr->current_info.main_excepid = 0;
        mgr->current_info.sub_excepid = 0;
        mgr->current_info.tm.tv_sec = 0;
        mgr->current_info.tm.tv_usec = 0;
        mgr->current_info.done = NULL;
        mgr->current_info.etype = 0;

        ret = memset_s(mgr->thread_name, THREAD_NAME_LEN, 0, THREAD_NAME_LEN);
        BB_CHECK_SECUREC(BB_CHECK_MEMSET, ret);
        mgr->initialized = BBOX_FALSE;
    }
    g_proxy_mgr.count = 0;
    bbox_product_proxy_exit();
    g_proxy_init = BBOX_FALSE;
    BB_PRINT_INFO("bbox proxy end exit.\n");

    return;
}
