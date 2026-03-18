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

#include "hdr_bootcheck.h"

#include <linux/securec.h>

#include "bootparam/bbox_boot_param.h"
#include "bbox_bootcheck_pub.h"
#include "common/bbox_common.h"
#include "common/bbox_print.h"
#include "communication/bbox_message.h"
#include "config/bbox_config.h"
#include "hdr_public.h"
#include "rdr/rdr_common.h"
#include "rdr/rdr_dump_core.h"
#include "rdr/rdr_module_core.h"

/*
 * @brief       : clear log exception flag
 * @param [in]  : struct hdr_log_data *data   hdr log data
 * @return      : NA
 */
static inline void hdr_bootcheck_clear_log(struct hdr_log_data *data)
{
    data->head.boot.region_ctrl.err_area_cnt = 0;
    data->head.run.region_ctrl.err_area_cnt = 0;
}

/*
 * @brief       : clear status exception flag
 * @param [in]  : struct hdr_status_data *data   hdr status data
 * @return      : NA
 */
static inline void hdr_bootcheck_clear_status(struct hdr_status_data *data)
{
    u32 i;
    for (i = 0; i < (u32)BBOX_MIN(STATUS_AREA_MAX_NUM, data->head.region.cfg.used_area_num); i++) {
        // only process history
        if (data->head.region.ctrl.area_ctrl[i].type == STATUS_AREA_HISTORY) {
            data->head.region.ctrl.area_ctrl[i].rw_flag = STATUS_AREA_READ_DONE;
        }
    }
}

/*
 * @brief       : send message success, callback function
 * @param [in]  : void *arg         device id
 * @return      : NA
 */
STATIC void hdr_bootcheck_notify(void *arg)
{
    s32 ret;
    u32 devid;
    u64 addr, size;
    struct hdr_data *data = NULL;

    BB_CHECK_PTR(arg, return, "invalid parameter, arg is NULL.\n");

    // get config
    ret = bbox_config_get_hdr(&addr, &size);
    if ((ret != BBOX_SUCCESS) || (addr == 0U) || (size < sizeof(struct hdr_data))) {
        BB_PRINT_ERR("get hdr ddr addr failed.\n");
        return;
    }

    devid = *(u32 *)arg;    // arg is malloced and needs to be free.
    data = (struct hdr_data *)bbox_ioremap(bbox_get_device_addr(devid, addr), size);
    if (data == NULL) {
        BB_PRINT_ERR("map hdr ddr addr failed.\n");
        return;
    }

    // process log
    if (data->log_data.head.magic == HDR_LOG_MAGIC) {
        hdr_bootcheck_clear_log(&data->log_data);
    }

    // process status
    if (data->status.head.magic == HDR_STATUS_MAGIC) {
        hdr_bootcheck_clear_status(&data->status);
    }

    bbox_iounmap(data);
}

/*
 * @brief       : get run excepid
 * @param [in]  : struct hdr_log_head *head     hdr head
 * @param [in]  : u32 area_index                area index
 * @param [in]  : u8 coreid                     module id
 * @return      : ==0 failure; >0 exception id
 */
#ifndef BBOX_SOC_PLATFORM_MDC
STATIC u32 hdr_bootcheck_get_run_log_excepid(const struct hdr_log_head *head, u32 area_index, u8 coreid)
{
    s32 i;
    u32 etype;
    u32 excepid = 0;

    BB_CHECK_PTR(head, return 0, "invalid param, head is NULL.\n");
    BB_CHECK_EXP_ACT(area_index >= AREA_MAX_NUM, return 0, "invalid param, area index is %u.\n", area_index);

    etype = head->run.region_ctrl.area_ctrl[area_index].e_type;
    // special handling, for some exception introduced with running
    if ((etype == (u32)PMU_EXCEPTION) || (etype == (u32)TSENSOR_EXCEPTION) ||
        (etype == (u32)ABNORMAL_EXCEPTION) || (etype == (u32)DEVICE_LTO_EXCEPTION)) {
        return bbox_bootcheck_get_excepid((u8)etype);
    }

    for (i = 0; i < (s32)BBOX_MIN(MODULE_MAX_NUM, head->run.region_cfg.areas.used_module_num); i++) {
        const struct module_head *module = NULL;
        if (head->run.region_cfg.areas.module_que[i].size == 0) {
            continue;
        }
        module = hdr_log_get_run_module((const char *)head, &head->run, area_index, i);
        if ((module != NULL) && (module->module_id == coreid)) {
            excepid = module->err_code;
            break;
        }
    }

    if (bbox_excepid_check(excepid) == true) {
        return excepid;
    } else {
        // use default when cannot match
        return hdr_bootcheck_gen_excepid(coreid);
    }
}
#endif
/*
 * @brief       : send to remote
 * @param [in]  : struct rdr_exception_msg_info *info       send info
 * @param [in]  : struct hdr_data *data                     data
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 hdr_bootcheck_send_data(const struct rdr_exception_msg_info *info, const struct hdr_data *data)
{
    BB_CHECK_PTR(info, return BBOX_FAILURE, "invalid param, info is NULL.\n");
    BB_CHECK_PTR(data, return BBOX_FAILURE, "invalid param, data is NULL.\n");

    rdr_save_history_log_for_bootcheck(info);
    return bbox_submit_hdr_message(info, hdr_bootcheck_notify, (const char *)data, sizeof(struct hdr_data));
}

/*
 * @brief       : get boot log exception for send info
 * @param [in]  : struct hdr_log_data *data                 hdr log data
 * @param [out] : struct hdr_exception_info *info           send info
 * @return      : NA
 */
STATIC void hdr_bootcheck_process_boot_log(const struct hdr_log_data *data, struct hdr_exception_info *info)
{
    s32 i;

    if (data->head.boot.region_ctrl.err_area_cnt == 0) {
        return;
    }

    for (i = 0; i < (s32)BBOX_MIN(data->head.boot.region_cfg.total_area_num, AREA_MAX_NUM); i++) {
        if ((data->head.boot.region_ctrl.area_ctrl[i].flag == (u32)AREA_IN_DDR_ERR_Q) &&
            (info->reset_num > data->head.boot.region_ctrl.area_ctrl[i].reset_num)) {
            info->reset_num = data->head.boot.region_ctrl.area_ctrl[i].reset_num;
#ifndef BBOX_SOC_PLATFORM_MDC
            info->coreid = hdr_bootcheck_gen_coreid(data->head.boot.region_ctrl.area_ctrl[i].module_id, BBOX_BIOS);
            info->etype = (u8)HDR_EXCEPTION;
            info->excepid = hdr_bootcheck_gen_boot_excepid(data->head.boot.region_ctrl.area_ctrl[i].err_code,
                                                           info->coreid);
#else
            info->coreid = BBOX_COMMON;
            info->etype = BOOT_DOT_INFO;
            info->excepid = BOOT_DOT_ID;
#endif
        }
    }
}

/*
 * @brief       : get run log exception for send info
 * @param [in]  : struct hdr_log_data *data         hdr log data
 * @param [out] : struct hdr_exception_info *info   send info
 * @return      : NA
 */
STATIC void hdr_bootcheck_process_run_log(const struct hdr_log_data *data, struct hdr_exception_info *info)
{
    s32 i;

    if (data->head.run.region_ctrl.err_area_cnt == 0) {
        return;
    }

    for (i = 0; i < (s32)BBOX_MIN(data->head.run.region_cfg.total_area_num, AREA_MAX_NUM); i++) {
        if ((data->head.run.region_ctrl.area_ctrl[i].flag == (u32)AREA_IN_DDR_ERR_Q) &&
            (info->reset_num > data->head.run.region_ctrl.area_ctrl[i].reset_num)) {
            info->reset_num = data->head.run.region_ctrl.area_ctrl[i].reset_num;
#ifndef BBOX_SOC_PLATFORM_MDC
            info->coreid = bbox_bootcheck_get_coreid(data->head.run.region_ctrl.area_ctrl[i].e_type);
            info->etype = (u8)HDR_EXCEPTION;
            info->excepid = hdr_bootcheck_get_run_log_excepid(&data->head, (u32)i, info->coreid);
#else
            info->coreid = BBOX_COMMON;
            info->etype = BOOT_DOT_INFO;
            info->excepid = BOOT_DOT_ID;
#endif
        }
    }
}

/*
 * @brief       : get log exception for send info
 * @param [in]  : u32 devid                         device id
 * @param [in]  : struct hdr_log_data *data         hdr log data
 * @param [out] : struct hdr_exception_info *info   send info
 * @return      : NA
 */
STATIC void hdr_bootcheck_process_log(u32 devid,
                                      const struct hdr_log_data *data,
                                      struct hdr_exception_info *info)
{
    if (data->head.magic != HDR_LOG_MAGIC) {
        BB_PRINT_INFO("[device-%u] hdr log feature is unusable. (magic=%u)\n", devid, data->head.magic);
        return;
    }

    if ((data->head.boot.region_ctrl.err_area_cnt == 0) &&
        (data->head.run.region_ctrl.err_area_cnt == 0)) {
        BB_PRINT_INFO("[device-%u] hdr log, no has exception data.\n", devid);
        return;
    }

    hdr_bootcheck_process_boot_log(data, info);
    hdr_bootcheck_process_run_log(data, info);
}

/*
 * @brief       : check status area block data
 * @param [in]  : struct status_block *block    block data
 * @return      : NA
 */
static inline void hdr_bootcheck_status_block(const struct status_block *block)
{
    bool ret = bbox_excepid_check(block->exception_id);
    if (!ret) {
        BB_PRINT_ERR("invlid exception id[0x%x] on block[%hhu].\n", block->exception_id, block->block_id);
    }
}

#if (defined BBOX_SOC_PLATFORM_MDC && !defined BBOX_UT)
STATIC struct hdr_lastbootstatus g_bboxlastbootstatus[BBOX_BOOT_STATUS_BLOCK_MAX_NUM] = {0};

s32 bbox_getLastBootState(unsigned int devId, unsigned int key, unsigned int *state)
{
    u32 i;
    *state = 0;

    for (i = 0; i < BBOX_BOOT_STATUS_BLOCK_MAX_NUM; i++) {
        if (g_bboxlastbootstatus[i].stageid == key) {
            if (g_bboxlastbootstatus[i].result != 0) {
                *state = g_bboxlastbootstatus[i].result;
            }
        }
    }
    BB_PRINT_INFO("Get last boot state info. (key=%u, state=%u)", key, *state);
    return BBOX_SUCCESS;
}
EXPORT_SYMBOL(bbox_getLastBootState);

STATIC void hdr_bootcheck_print_lastbootstatus(void)
{
    u32 num = sizeof(g_bboxlastbootstatus) / sizeof(g_bboxlastbootstatus[0]), i;
    for (i = 0; i < num; i++) {
        BB_PRINT_INFO("Last boot status info. (seq=%u, block_id=%u, current=%u, stageid=%u, result=%u)\n",
            i, g_bboxlastbootstatus[i].blockid, g_bboxlastbootstatus[i].currentstatus,
            g_bboxlastbootstatus[i].stageid, g_bboxlastbootstatus[i].result);
    }
}

STATIC void hdr_bootcheck_bootstatussubproc(const struct bbox_config_bootstatus *bootstatus,
                                            struct status_block block, u32 k, u32 *idx)
{
    u32 m;

    if ((bootstatus[k].block_id != block.block_id) ||
        (bootstatus[k].file_num == 0) ||
        ((*idx) >= BBOX_BOOT_STATUS_BLOCK_MAX_NUM)) {
        return;
    }
    g_bboxlastbootstatus[*idx].blockid = bootstatus[k].block_id;
    g_bboxlastbootstatus[*idx].currentstatus = block.current_status;
    g_bboxlastbootstatus[*idx].stageid = bootstatus[k].stage_id;
    for (m = 0; m < bootstatus[k].file_num; m++) {
        if ((block.current_status > bootstatus[k].file_list[m].min_valid_value) &&
            (block.current_status < bootstatus[k].file_list[m].max_valid_value)) {
            g_bboxlastbootstatus[*idx].result = bootstatus[k].file_list[m].file_id;
        }
    }
    (*idx)++;
}

STATIC void hdr_bootcheck_bootstatusproc(const struct bbox_config_bootstatus *bootstatus,
                                         struct status_block block, u32 num, u32 *idx)
{
    u32 k;
    for (k = 0; k < num; k++) {
        hdr_bootcheck_bootstatussubproc(bootstatus, block, k, idx);
    }
}

STATIC void hdr_bootcheck_rootfscmsproc(struct bbox_config_rootfscms rootfscms,
                                        struct status_block block, u32 *idx)
{
    if ((rootfscms.block_id != block.block_id) || (*idx >= BBOX_BOOT_STATUS_BLOCK_MAX_NUM)) {
        return;
    }
    g_bboxlastbootstatus[*idx].blockid = rootfscms.block_id;
    g_bboxlastbootstatus[*idx].currentstatus = block.current_status;
    g_bboxlastbootstatus[*idx].stageid = rootfscms.stage_id;
    if ((block.current_status > rootfscms.min_valid_value) &&
        (block.current_status < rootfscms.max_valid_value)) {
        g_bboxlastbootstatus[*idx].result = rootfscms.result;
    }
    (*idx)++;
}

/*
 * @brief       : Get last boot status
 * @param [in]  : struct status_block *block    block data
 * @return      : NA
 */
STATIC void hdr_bootcheck_getLastBootStatus(u32 devid, struct hdr_status_data *data)
{
    u32 i, j, reset_num, num, idx = 0;
    const struct bbox_config_bootstatus *bootstatus = bbox_get_config_bootstatus(&num);
    const struct bbox_config_rootfscms rootfscms = bbox_get_config_rtoofscms();

    if (data->head.magic != HDR_STATUS_MAGIC) {
        BB_PRINT_INFO("Hdr status feature is unusable.\n");
        return;
    }

    for (i = 0; i < BBOX_MIN(STATUS_AREA_MAX_NUM, data->head.region.cfg.used_area_num); i++) {
        if (data->head.region.ctrl.area_ctrl[i].type == STATUS_AREA_CURRENT) {
            reset_num = data->head.region.ctrl.area_ctrl[i].reset_num;
            continue;
        }

        if (data->head.region.ctrl.area_ctrl[i].reset_num != (reset_num - 1)) {
            continue;
        }
        BB_PRINT_INFO("Hdr bootcheck last boot info. (reset_num=%u, total_block_num=%u)\n",
                      (reset_num - 1), data->head.region.cfg.total_block_num);
        for (j = 0; j < BBOX_MIN(STATUS_BLOCK_MAX_NUM, data->head.region.cfg.total_block_num); j++) {
            hdr_bootcheck_rootfscmsproc(rootfscms, data->block[i][j], &idx);
            hdr_bootcheck_bootstatusproc(bootstatus, data->block[i][j], num, &idx);
        }
    }
    hdr_bootcheck_print_lastbootstatus();
}
#endif

/*
 * @brief       : get status exception for send info
 * @param [in]  : u32 devid                         device id
 * @param [in]  : struct hdr_status_data *data      hdr status data
 * @param [out] : struct hdr_exception_info *info   send info
 * @return      : NA
 */
STATIC void hdr_bootcheck_process_status(u32 devid,
                                         struct hdr_status_data *data,
                                         struct hdr_exception_info *info)
{
    u32 i, j;
    bool is_exp = false;

    if (data->head.magic != HDR_STATUS_MAGIC) {
        BB_PRINT_INFO("[device-%u] hdr status feature is unusable.\n", devid);
        return;
    }

    for (i = 0; i < BBOX_MIN(STATUS_AREA_MAX_NUM, data->head.region.cfg.used_area_num); i++) {
        // process area
        if (data->head.region.ctrl.area_ctrl[i].type != STATUS_AREA_HISTORY) {
            continue;
        }
        if (data->head.region.ctrl.area_ctrl[i].rw_flag != STATUS_AREA_WRITE_DONE) {
            continue;
        }

        // process block
        for (j = 0; j < BBOX_MIN(STATUS_BLOCK_MAX_NUM, data->head.region.cfg.total_block_num); j++) {
            // block invalid
            if (data->block[i][j].valid != STATUS_BLOCK_VALID) {
                continue;
            }
            hdr_bootcheck_status_block(&data->block[i][j]);

#if !defined(BBOX_SOC_PLATFORM_MDC) && !defined(BBOX_SOC_PLATFORM_MDC_V11)
            // block no have exception
            if (data->block[i][j].expect_status == data->block[i][j].current_status) {
                continue;
            }
            data->head.region.ctrl.area_ctrl[i].exception = STATUS_AREA_EXCEPTION;
#else
            if (data->block[i][j].expect_status != data->block[i][j].current_status) {
                is_exp = true;
                data->head.region.ctrl.area_ctrl[i].exception = STATUS_AREA_EXCEPTION;
            }
#endif
            // get exception info
            if (info->reset_num > data->head.region.ctrl.area_ctrl[i].reset_num) {
                info->reset_num = data->head.region.ctrl.area_ctrl[i].reset_num;
#if !defined(BBOX_SOC_PLATFORM_MDC) && !defined(BBOX_SOC_PLATFORM_MDC_V11)
                info->excepid = data->block[i][j].exception_id;
                info->coreid = bbox_excepid_get_coreid(data->block[i][j].exception_id);
                info->etype = (u8)HDR_EXCEPTION;
                is_exp = true;
#else
                info->excepid = BOOT_DOT_ID;
                info->coreid = BBOX_COMMON;
                info->etype = BOOT_DOT_INFO;
#endif
            }
        }
    }
    if (is_exp == false) {
        BB_PRINT_INFO("[device-%u] hdr status, no has exception data.\n", devid);
        hdr_bootcheck_clear_status(data);
    }
}

/*
 * @brief       : get status exception for send info
 * @param [in]  : u32 devid                         device id
 * @param [in]  : struct bbox_time *tm              time
 * @param [out] : struct hdr_data *data             data
 * @return      : NA
 */
STATIC void hdr_bootcheck_process_device(u32 devid, const struct bbox_time *tm, struct hdr_data *data)
{
    s32 ret;
    struct rdr_exception_msg_info msg_info;
    struct hdr_exception_info info = {BBOX_UINT_INVALID, 0, BBOX_UNDEF, BBOX_EXCEPTION_REASON_INVALID};

    hdr_bootcheck_process_status(devid, &data->status, &info);
    hdr_bootcheck_process_log(devid, &data->log_data, &info);
#if (defined BBOX_SOC_PLATFORM_MDC && !defined BBOX_UT)
    hdr_bootcheck_getLastBootStatus(devid, &data->status);
#endif

    // determine whether an exception occurs based on the number of resets.
    if (info.reset_num == BBOX_UINT_INVALID) {
        return;
    }

    // geted exception info
    msg_info.coreid = info.coreid;
    msg_info.excepid = info.excepid;
    msg_info.etype = info.etype;
    msg_info.devid = devid;
    msg_info.tm.tv_sec = tm->tv_sec;
    msg_info.tm.tv_nsec = tm->tv_nsec;

    // send data
    ret = hdr_bootcheck_send_data(&msg_info, data);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("[device-%u] hdr bootcheck send data failed.\n", devid);
    }
}

/*
 * @brief       : history data record process
 * @param [in]  : struct bbox_time *start       start time
 * @return      : !=0 failure; ==0 success
 */
s32 hdr_bootcheck_process(const struct bbox_time *start)
{
    s32 i, ret;
    u64 addr, size;
    struct hdr_data *data = NULL;
    struct bbox_time tm;

    BB_CHECK_PTR(start, return BBOX_FAILURE, "invalid param, start is NULL.\n");

    ret = bbox_config_get_hdr(&addr, &size);
    BB_CHECK_EXP_CTRL(BB_PRINT_ERR, ret != BBOX_SUCCESS, return BBOX_FAILURE, "get hdr config failed.\n");

    if ((addr == 0) || (size == 0)) {
        BB_PRINT_INFO("bbox hdr bootcheck is closed.\n");
        return BBOX_SUCCESS;
    }

    if (size < sizeof(struct hdr_data)) {
        BB_PRINT_INFO("bbox hdr space is too small.\n");
        return BBOX_FAILURE;
    }

    tm.tv_sec = start->tv_sec;
    tm.tv_nsec = start->tv_nsec;
    bbox_update_time_seq(&tm);
    for (i = 0; i < (s32)bbox_get_device_num(); i++) {
        data = (struct hdr_data *)bbox_ioremap(bbox_get_device_addr((u32)i, addr), size);
        if (data == NULL) {
            BB_PRINT_ERR("[device-%d] map hdr space failed.\n", i);
            continue;
        }

        hdr_bootcheck_process_device((u32)i, &tm, data);
        bbox_iounmap((const void *)data);
        data = NULL;
    }

    return BBOX_SUCCESS;
}

