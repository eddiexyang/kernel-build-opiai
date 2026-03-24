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

#include "bbox_adapter_snapshot.h"

#include <asm/current.h>
#include <asm/traps.h>
#include <linux/ftrace.h>
#include <linux/kdebug.h>
#include <linux/securec.h>
#include <linux/sched.h>

#include "bbox_adapter.h"
#include "bbox_platform.h"
#include "bootcheck/hdr_bootcheck.h"
#include "common/bbox_print.h"
#include "common/bbox_sys_api.h"
#include "config/bbox_config.h"
#include "device/bbox_types.h"
#include "rdr/rdr_common.h"
#include "rdr/rdr_field_core.h"

STATIC bbox_bit_t g_event_flag = 0;
STATIC DEFINE_SPINLOCK(g_event_lock);
STATIC bbox_bit_t g_dump_flag = 0;
STATIC DEFINE_SPINLOCK(g_event_dump_lock);
STATIC bool g_snapshot_init = false;
STATIC struct snapshot_buf_cb *g_snapshot_buf_cb = NULL;

#define RUN_MODULE_OS_SIZE                0x1000U

/*
 * @brief       : pre process
 * @param [in]  : enum EVENT_TYPE event         event, exp: die,panic
 * @return      : EVENT_FLAG_INVALID: 无效事件
 *                EVENT_FLAG_NONE: 无并发事件
 *                EVENT_FLAG_NEST: 异常嵌套
 *                EVENT_FLAG_CALL: 一个异常事件被另一个异常事件调用
 */
STATIC s32 bbox_snapshot_pre_process(enum EVENT_TYPE event)
{
    s32 ret;
    lock_flag_t flags = 0;
    bool is_locked = true;

    is_locked = (spin_is_locked(&g_event_lock) != 0);
    if (is_locked) {
        BB_PRINT_ERR("event(%d) occurred concurrency.\n", (s32)event);
        return EVENT_FLAG_INVALID;
    }

    spin_lock_irqsave(&g_event_lock, flags);
    // same event
    if (test_bit((s32)event, &g_event_flag) != 0) {
        spin_unlock_irqrestore(&g_event_lock, flags);
        return EVENT_FLAG_NEST;
    }

    if ((event < EVENT_TYPE_PANIC) || (event >= EVENT_TYPE_MAX)) {
        spin_unlock_irqrestore(&g_event_lock, flags);
        BB_PRINT_ERR("unknown event[%d].", (s32)event);
        return EVENT_FLAG_INVALID;
    }

    if ((event == EVENT_TYPE_PANIC) && (test_bit((s32)EVENT_TYPE_DIE, &g_event_flag) != 0)) {
        ret = EVENT_FLAG_CALL;
    } else if ((event == EVENT_TYPE_DIE) && (g_event_flag != 0)) {
        ret = EVENT_FLAG_CALL;
    } else {
        ret = EVENT_FLAG_NONE;
    }

    set_bit((u32)event, &g_event_flag);
    spin_unlock_irqrestore(&g_event_lock, flags);
    return ret;
}

/*
 * @brief       : post process
 * @param [in]  : enum EVENT_TYPE event         event, exp: die,panic
 * @return      : != 0: failure; == 0: success
 */
STATIC void bbox_snapshot_post_process(enum EVENT_TYPE event)
{
    bool is_locked = (spin_is_locked(&g_event_dump_lock) != 0);
    if (is_locked) {
        BB_PRINT_ERR("event(%d) occurred concurrency.\n", (s32)event);
        return;
    }

    if ((event >= EVENT_TYPE_PANIC) && (event < EVENT_TYPE_MAX)) {
        lock_flag_t flags = 0;
        spin_lock_irqsave(&g_event_dump_lock, flags);
        set_bit((u32)event, &g_dump_flag);
        spin_unlock_irqrestore(&g_event_dump_lock, flags);
    }
}

/*
 * @brief       : print a message to logbuf
 * @param [in]  : struct snapshot_buf_cb *cb    buf control block
 * @param [in]  : const char *fmt               format string
 * @param [in]  : va_list args                  the variables for fmt string
 * @return      : -1: failure; >=0: the number of chars printed
 */
STATIC s32 bbox_snapshot_vprint(struct snapshot_buf_cb *cb, const char *fmt, va_list args)
{
    s32 i;
    s32 ret;
    s32 logged_chars = 0;

    // Copy the output into the temporary buffer
    ret = vsnprintf_s(cb->printk_buf, SNAPSHOT_PRINT_LEN, SNAPSHOT_PRINT_LEN - 1, fmt, args);
    if (ret == -1) {
        BB_PRINT_ERR("vsnprintf_s failed with %d.\n", ret);
        return BBOX_FAILURE;
    }

    // Copy the output into log_buf.
    for (i = 0; i < ret; i++) {
        if ((cb->log_end - cb->log_start) >= cb->log_buf_len) {
            cb->full = BBOX_TRUE;
            break;
        }
        cb->log_buf[cb->log_end] = cb->printk_buf[i];
        cb->log_end++;
        logged_chars++;
    }

    return logged_chars;
}

/*
 * @brief       : print a message to log buf
 *                一次最多只能打印256字节的数据，以后的内容会被丢失，调用者需要保证这一点
 * @param [in]  : struct snapshot_buf_cb *cb    logbuf control block
 * @param [in]  : const char *fmt               format string
 * @return      : >=0:the number of chars printed; -1:error
 */
STATIC s32 bbox_snapshot_print(struct snapshot_buf_cb *cb, const char *fmt, ...)
{
    s32 ret;
    va_list args;

    BB_CHECK_PTR(cb, return BBOX_FAILURE, "invalid param, cb is NULL.\n");
    BB_CHECK_PTR(fmt, return BBOX_FAILURE, "invalid param, fmt is NULL.\n");

    if ((cb->log_end - cb->log_start) >= cb->log_buf_len) {
        BB_PRINT_INFO("log buffer is full.\n");
        cb->err++;
        cb->full = BBOX_TRUE;
        return BBOX_FAILURE;
    }

    ret = memset_s(&args, sizeof(va_list), 0, sizeof(va_list));
    if (ret != EOK) {
        BB_PRINT_ERR("memset_s failed, ret:%d", ret);
    }

    va_start(args, fmt);
    ret = bbox_snapshot_vprint(cb, fmt, args);
    va_end(args);

    if ((ret < 0) || (cb->full == BBOX_TRUE)) {
        cb->err++;
    }
    return ret;
}

/*
 * @brief       : print process info to log buf
 * @param [in]  : struct snapshot_buf_cb *cb    logbuf control block
 * @param [in]  : struct task_struct *task      task
 * @return      : != 0: failure; == 0: success
 */
STATIC void bbox_snapshot_dump_process(struct snapshot_buf_cb *cb, const struct task_struct *task)
{
    s32 ret;

    BB_CHECK_PTR(cb, return, "invalid param, cb is NULL.\n");
    BB_CHECK_PTR(task, return, "invalid param, task is NULL.\n");

    ret = bbox_snapshot_print(cb, "process: %s (pid: %d, tgid: %d) on CPU: %d\n",
                              task->comm, task->pid, task->tgid, smp_processor_id());
    if (ret < 0) {
        BB_PRINT_ERR("print process info failed with %d.\n", ret);
    }
}

/*
 * @brief       : print panic time and reason to log buf
 * @param [in]  : struct snapshot_buf_cb *cb    logbuf control block
 * @param [in]  : char *event_name              event name: panic/die
 * @return      : NA
 */
STATIC void bbox_snapshot_dump_time(struct snapshot_buf_cb *cb, const char *event_name)
{
    s32 ret;
    char datetime[SNAPSHOT_DATE_LENGTH] = {0};
    struct bbox_time curr = {0, 0};

    BB_CHECK_PTR(cb, return, "invalid param, cb is NULL.\n");
    BB_CHECK_PTR(event_name, return, "invalid param, event name is NULL.\n");

    bbox_get_safe_systime(&curr);
    bbox_get_sysdate(&curr, datetime, SNAPSHOT_DATE_LENGTH);
    ret = bbox_snapshot_print(cb, "%s process time:%s UTC\n", event_name, datetime);
    if (ret < 0) {
        BB_PRINT_ERR("print panic time failed with %d.\n", ret);
    }

    return;
}

/*
 * @brief       : print panic reason to log buf
 * @param [in]  : struct snapshot_buf_cb *cb    logbuf control block
 * @param [in]  : char *panic_reason            panic reason string
 * @return      : NA
 */
STATIC void bbox_snapshot_dump_panic_reason(struct snapshot_buf_cb *cb, const char *panic_reason)
{
    s32 ret;
    const char *reason = ((panic_reason != NULL) ? panic_reason : PANIC_REASON_UNKOW);

    BB_CHECK_PTR(cb, return, "invalid param, cb is NULL.\n");
    ret = bbox_snapshot_print(cb, "panic reason:%s\n", reason);
    if (ret < 0) {
        BB_PRINT_ERR("print panic reason failed with %d.\n", ret);
    }
}

/*
 * @brief       : print die info to log buf
 * @param [in]  : struct snapshot_buf_cb *cb    logbuf control block
 * @param [in]  : struct die_args *args         die args
 * @return      : NA
 */
STATIC void bbox_snapshot_dump_die_reason(struct snapshot_buf_cb *cb, const char *str, u64 err)
{
    s32 ret;

    BB_CHECK_PTR(cb, return, "invalid param, cb is NULL.\n");
    ret = bbox_snapshot_print(cb, "die info:%s:%04lx\n",
                              ((str == NULL) ? "unknown" : str),
                              (err & DIE_ERR_MASK));
    if (ret < 0) {
        BB_PRINT_ERR("print panic die info failed with %d.\n", ret);
    }
}

/*
 * @brief       : set os block head
 * @param [in]  : struct snapshot_buf_cb *cb        logbuf control block
 * @param [in]  : u8 etype                          exception type
 * @param [in]  : u32 excepid                       exception id
 * @return      : != 0: failure; == 0: success
 */
STATIC s32 bbox_snapshot_set_block_head(struct snapshot_buf_cb *cb, u8 etype, u32 excepid)
{
    BB_CHECK_PTR(cb, return BBOX_FAILURE, "invalid param, cb is NULL.\n");
    BB_CHECK_PTR(cb->module, return BBOX_FAILURE, "invalid param, module is NULL.\n");

    cb->module->is_used = BBOX_TRUE;
    cb->module->err_code = excepid;
    cb->module->reason = (u32)etype;
    return BBOX_SUCCESS;
}

/*
 * @brief       : init os block head
 * @param [in]  : struct snapshot_buf_cb *cb        logbuf control block
 * @return      : != 0: failure; == 0: success
 */
STATIC s32 bbox_snapshot_init_block_head(struct snapshot_buf_cb *cb)
{
    BB_CHECK_PTR(cb, return BBOX_FAILURE, "invalid param, cb is NULL.\n");
    BB_CHECK_PTR(cb->module, return BBOX_FAILURE, "invalid param, module is NULL.\n");
    BB_CHECK_PTR(cb->head, return BBOX_FAILURE, "invalid param, head is NULL.\n");

    cb->module->magic = SNAPSHOT_MAGIC;
    cb->module->version = SNAPSHOT_VERSION;
    cb->module->module_id = BBOX_OS;
    cb->module->is_used = BBOX_FALSE;
    cb->module->reset_cnt = cb->head->reset_cnt;
    return BBOX_SUCCESS;
}

/*
 * @brief       : set os snapshot key
 * @param [in]  : struct snapshot_buf_cb *cb    logbuf control block
 * @return      : != 0: failure; == 0: success
 */
STATIC s32 bbox_snapshot_set_snapshot_key(const struct snapshot_buf_cb *cb)
{
    BB_CHECK_PTR(cb, return BBOX_FAILURE, "invalid param, cb is NULL.\n");
    BB_CHECK_PTR(cb->key, return BBOX_FAILURE, "invalid param, cb->key is NULL.\n");

    cb->key->event_flag = (u32)g_event_flag;
    cb->key->dump_flag = (u32)g_dump_flag;
    cb->key->err = cb->err;
    return BBOX_SUCCESS;
}

/*
 * @brief       : check hdr region info
 * @param [in]  : struct log_region_info *region    region
 * @param [in]  : u32 size                          hdr log size
 * @return      : != 0: failure; == 0: success
 */
STATIC s32 bbox_snapshot_check_region(const struct log_region_info *region, u32 size)
{
    const struct module_pos_info *os_pos_info = NULL;
    u32 area_size;
    u32 total_number;
    BB_CHECK_PTR(region, return BBOX_FAILURE, "invalid param, region is NULL.\n");

    if ((region->offset >= size) || ((size - region->offset) < region->size)) {
        BB_PRINT_ERR("Invalid offset or size of hdr region. (region->offset=%u, region->size=%u, size=%u)\n",
                     region->offset, region->size, size);
        return BBOX_FAILURE;
    }

    if (region->offset < sizeof(struct hdr_log_head)) {
        BB_PRINT_ERR("invalid offset of hdr region. (region->offset=%u)\n", region->offset);
        return BBOX_FAILURE;
    }

    total_number = region->region_cfg.total_area_num;
    if (total_number == 0) {
        BB_PRINT_ERR("invalid total area num of hdr region. (total_number=%u)\n", total_number);
        return BBOX_FAILURE;
    }

    area_size = region->size / total_number;
    if (area_size < RUN_MODULE_OS_SIZE) {
        BB_PRINT_ERR("invalid area size of hdr region. (area_size=%u)\n", area_size);
        return BBOX_FAILURE;
    }

    os_pos_info = &region->region_cfg.areas.module_que[RUN_MODULE_OS];
    if ((os_pos_info->offset >= area_size) || ((area_size - os_pos_info->offset) < os_pos_info->size)) {
        BB_PRINT_ERR("invalid offset or size of os module pos info. (os_pos_info->offset=%u, os_pos_info->size=%u, "
                     "area_size=%u)\n", os_pos_info->offset, os_pos_info->size, area_size);
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : check hdr magic
 * @param [in]  : struct hdr_log_head *head hdr head
 * @param [in]  : u32 size                  hdr size
 * @return      : != 0: failure; == 0: success
 */
STATIC s32 bbox_snapshot_check_hdr(const struct hdr_log_head *head, u32 size)
{
    BB_CHECK_PTR(head, return BBOX_FAILURE, "invalid param, head is NULL.\n");

    if (head->magic != HDR_LOG_MAGIC) {
        BB_PRINT_ERR("hdr ddr is unusable, magic(%u), version(%u).\n", head->magic, head->version);
        return BBOX_FAILURE;
    }

    if (bbox_snapshot_check_region(&head->run, size) != BBOX_SUCCESS) {
        BB_PRINT_ERR("run region of hdr ddr is invalid.\n");
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : init logbuf control block data
 * @param [in]  : u64 addr          buffer addr
 * @param [in]  : u32 size          buffer size
 * @return      : logbuf control block point
 */
STATIC struct snapshot_buf_cb *bbox_snapshot_buf_init(u64 addr, u32 size)
{
    s32 ret;
    struct snapshot_buf_cb *cb = NULL;
    struct hdr_log_head *head = NULL;
    struct module_head *md = NULL;

    head = (struct hdr_log_head *)bbox_ioremap((phys_addr_t)addr, size);
    BB_CHECK_EXP_ACT(head == NULL, return NULL, "map snapshot ddr addr failed.\n");

    // check hdr is usable
    ret = bbox_snapshot_check_hdr(head, size);
    if (ret != BBOX_SUCCESS) {
        bbox_iounmap(head);
        return NULL;
    }

    md = hdr_log_get_curr_run_module((const char *)head, &head->run, (s32)RUN_MODULE_OS);
    if ((md == NULL) ||
        ((uintptr_t)md <= (uintptr_t)head) ||
        (((uintptr_t)head + size) <= ((uintptr_t)md + RUN_MODULE_OS_SIZE))) {
        BB_PRINT_ERR("module addr is invalid.\n");
        bbox_iounmap(head);
        return NULL;
    }

    ret = memset_s(md, RUN_MODULE_OS_SIZE, 0, RUN_MODULE_OS_SIZE);
    if (ret != EOK) {
        BB_PRINT_ERR("memset_s os block failed with %d.\n", ret);
    }

    cb = (struct snapshot_buf_cb *)bbox_vmalloc(sizeof(struct snapshot_buf_cb));
    if (cb == NULL) {
        BB_PRINT_ERR("bbox vmalloc failed.\n");
        bbox_iounmap(head);
        return NULL;
    }

    cb->buffer = (char *)head;
    cb->size = size;
    cb->head = head;
    cb->module = md;
    cb->key = (struct snapshot_key *)(&((char *)md)[sizeof(struct module_head)]);
    cb->log_buf = (char *)(&((char *)md)[SNAPSHOT_LOG_BUF_OFFSET]);
    cb->log_buf_len = RUN_MODULE_OS_SIZE - SNAPSHOT_LOG_BUF_OFFSET;
    cb->log_start = 0;
    cb->log_end = 0;
    return cb;
}

/*
 * @brief       : reinit logbuf control block data
 * @param [in]  : struct snapshot_buf_cb *cb    logbuf control block
 * @return      : NA
 */
STATIC void bbox_snapshot_buf_reinit(struct snapshot_buf_cb *cb)
{
    size_t size = sizeof(struct snapshot_key);
    BB_CHECK_PTR(cb, return, "invalid param, cb is NULL.\n");
    (void)memset_s(cb->key, size, 0, size);
    (void)memset_s(cb->log_buf, cb->log_buf_len, 0, cb->log_buf_len);
    cb->log_start = 0;
    cb->log_end = 0;
}

/*
 * @brief       : clear snapshot buf control block
 * @param [in]  : struct snapshot_buf_cb *cb    logbuf control block
 * @return      : NA
 */
STATIC void bbox_snapshot_buf_clear(struct snapshot_buf_cb *cb)
{
    BB_CHECK_PTR(cb, return, "invalid param, cb is NULL.\n");

    cb->head = NULL;
    cb->key = NULL;
    cb->module = NULL;
    if (cb->buffer != NULL) {
        bbox_iounmap(cb->buffer);
        cb->buffer = NULL;
    }
    cb->size = 0;
    cb->log_start = 0;
    cb->log_end = 0;
    cb->log_buf_len = 0;
}

/*
 * @brief       : init snapshot modle
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_snapshot_init(void)
{
    s32 ret;
    u64 size = 0;
    u64 paddr = 0;

    if (g_snapshot_init == true) {
        return BBOX_SUCCESS;
    }

    ret = bbox_config_get_hdr(&paddr, &size);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("bbox config hdr get failed.\n");
        return BBOX_FAILURE;
    }

    if ((paddr == 0) || (size == 0)) {
        BB_PRINT_INFO("bbox OS snapshot is closed.\n");
        return BBOX_NO_SUPPORT;
    }

    g_snapshot_buf_cb = bbox_snapshot_buf_init(paddr, (u32)size);
    if (g_snapshot_buf_cb == NULL) {
        BB_PRINT_ERR("bbox snapshot init failed, OS snapshot off.\n");
        return BBOX_NO_SUPPORT;
    }

    // set module block head
    ret = bbox_snapshot_init_block_head(g_snapshot_buf_cb);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("os snapshot ddr init failed with %d.\n", ret);
    }

    g_snapshot_init = true;
    return BBOX_SUCCESS;
}

/*
 * @brief       : exit snapshot modle
 * @return      : NA
 */
void bbox_snapshot_exit(void)
{
    g_snapshot_init = false;
    if (g_snapshot_buf_cb != NULL) {
        bbox_snapshot_buf_clear(g_snapshot_buf_cb);
        BBOX_VFREE(g_snapshot_buf_cb);
    }
}

/*
 * @brief       : dump die snapshot data
 * @param [in]  : void *args     die args
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_snapshot_dump_die(const void *args)
{
    s32 ret, state;
    const struct die_args *info = (const struct die_args *)args;

    BB_CHECK_PTR(args, return BBOX_FAILURE, "die callback failed. invalid input param.\n");

    state = bbox_snapshot_pre_process(EVENT_TYPE_DIE);
    if (state != EVENT_FLAG_NONE) {
        BB_PRINT_INFO("bbox snapshot processed, state = %d.\n", state);
        return BBOX_SUCCESS;
    }

    if (g_snapshot_init == false) {
        return BBOX_SUCCESS;
    }

    bbox_snapshot_buf_reinit(g_snapshot_buf_cb);
    bbox_snapshot_dump_time(g_snapshot_buf_cb, EVENT_NAME_DIE);
    bbox_snapshot_dump_die_reason(g_snapshot_buf_cb, info->str, (u64)info->err);
    bbox_snapshot_dump_process(g_snapshot_buf_cb, current);

    bbox_snapshot_post_process(EVENT_TYPE_DIE);

    ret = bbox_snapshot_set_snapshot_key(g_snapshot_buf_cb);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("bbox snapshot set snapshot key failed with %d.\n", ret);
    }

    if ((g_snapshot_buf_cb->full == BBOX_TRUE) || (g_snapshot_buf_cb->err > 0)) {
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : dump panic snapshot data
 * @param [in]  : void *args        panic args
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_snapshot_dump_panic(const void *args)
{
    s32 ret, state;
    const char *reason = (const char *)args;

    state = bbox_snapshot_pre_process(EVENT_TYPE_PANIC);
    if (state == EVENT_FLAG_NEST) {
        BB_PRINT_INFO("bbox snapshot processed, state = %d.\n", state);
        return BBOX_SUCCESS;
    }

    if (g_snapshot_init == false) {
        BB_PRINT_INFO("Bbox snapshot has not inited.\n");
        return BBOX_SUCCESS;
    }

    // set module block head
    ret = bbox_snapshot_set_block_head(g_snapshot_buf_cb, OS_PANIC, (u32)EXCEPID_AP_PANIC);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("bbox snapshot set block panic failed with %d.\n", ret);
    }

    // other event has dumped data, panic event not exporting data
    if (state == EVENT_FLAG_CALL) {
        BB_PRINT_INFO("bbox snapshot processed, state = %d.\n", state);
        return BBOX_SUCCESS;
    }

    // dump panic data
    bbox_snapshot_buf_reinit(g_snapshot_buf_cb);
    bbox_snapshot_dump_time(g_snapshot_buf_cb, EVENT_NAME_PANIC);
    bbox_snapshot_dump_panic_reason(g_snapshot_buf_cb, reason);
    bbox_snapshot_dump_process(g_snapshot_buf_cb, current);

    bbox_snapshot_post_process(EVENT_TYPE_PANIC);

    ret = bbox_snapshot_set_snapshot_key(g_snapshot_buf_cb);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("bbox snapshot set snapshot key failed with %d.\n", ret);
    }

    if ((g_snapshot_buf_cb->full == BBOX_TRUE) || (g_snapshot_buf_cb->err > 0)) {
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : dump rdr base info
 * @param [in]  : struct snapshot_buf_cb *cb        logbuf control block
 * @param [in]  : struct bbox_exception_info *info  rdr base info
 * @param [in]  : struct bbox_time *tm              time
 * @return      : !=0 failure; ==0 success
 */
STATIC void bbox_snapshot_dump_comm_info(struct snapshot_buf_cb *cb,
                                         const struct bbox_exception_info *info,
                                         const struct bbox_time *tm)
{
    s32 ret;
    char date[DATATIME_MAXLEN] = {0};

    BB_CHECK_PTR(cb, return, "invalid input param, cb is NULL.\n");
    BB_CHECK_PTR(info, return, "invalid input param, info is NULL.\n");
    BB_CHECK_PTR(tm, return, "invalid input param, time is NULL.\n");

    bbox_get_date(tm, date, DATATIME_MAXLEN);
    ret = bbox_snapshot_print(cb,
        "base info:\n"
        "  excepid: 0x%x\n"
        "  core:    0x%x\n"
        "  eType:   0x%x\n"
        "  module:  %s\n"
        "  desc:    %s\n"
        "  date:    %s UTC\n\n",
        info->e_excepid, info->e_from_core, info->e_exce_type,
        (char *)info->e_from_module, (char *)info->e_desc, date);
    BB_CHECK_RET(ret < 0, "print rdr base info failed with %d.\n", ret);
}

/*
 * @brief       : dump hdc snapshot data
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_snapshot_dump_comm(const struct bbox_exception_info *info, const struct bbox_time *tm)
{
    s32 ret;

    if (g_snapshot_init == false) {
        return BBOX_SUCCESS;
    }

    // set module block head
    ret = bbox_snapshot_set_block_head(g_snapshot_buf_cb, OS_COMM, (u32)EXCEPID_AP_COMM);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("bbox snapshot set block panic failed with %d.\n", ret);
    }

    bbox_snapshot_buf_reinit(g_snapshot_buf_cb);
    bbox_snapshot_dump_time(g_snapshot_buf_cb, EVENT_NAME_COMM);
    bbox_snapshot_dump_comm_info(g_snapshot_buf_cb, info, tm);

    ret = bbox_snapshot_set_snapshot_key(g_snapshot_buf_cb);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("bbox snapshot set snapshot key failed with %d.\n", ret);
    }

    if ((g_snapshot_buf_cb->full == BBOX_TRUE) || (g_snapshot_buf_cb->err > 0)) {
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

