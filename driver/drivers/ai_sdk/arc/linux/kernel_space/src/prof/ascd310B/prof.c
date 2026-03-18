/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef PROF_UNIT_TEST

#include "prof.h"
#include "prof_drv_dev.h"
#ifdef CFG_FEATURE_SRIOV
#include "devdrv_interface.h"
#endif
int prof_hash[CHANNEL_IDS_MAX] = {
    [CHANNEL_DDR] = PROF_VALID_PERIPHERAL_FLAG,
    [CHANNEL_LLC] = PROF_VALID_PERIPHERAL_FLAG,
    [CHANNEL_TSCPU] = PROF_VALID_TSCPU_FLAG,

    [CHANNEL_TSFW] = PROF_VALID_TSCPU_SW_VIRTUAL_FLAG,
    [CHANNEL_TSFW_L2] = PROF_VALID_TSCPU_FLAG,
    [CHANNEL_STARS_SOC_PROFILE_BUFFER] = PROF_VALID_TSCPU_FLAG,
    [CHANNEL_STARS_SOC_LOG_BUFFER] = PROF_VALID_TSCPU_HW_VIRTUAL_FLAG,
    [CHANNEL_FFTS_PROFILE_BUFFER_TASK] = PROF_VALID_TSCPU_HW_VIRTUAL_FLAG,
    [CHANNEL_FFTS_PROFILE_BUFFER_SAMPLE] = PROF_VALID_TSCPU_FLAG,

#ifdef CFG_FEATURE_RC_MODE
    [CHANNEL_NIC] = PROF_VALID_PERIPHERAL_FLAG,
#endif
    [CHANNEL_DVPP] = PROF_VALID_PERIPHERAL_VIRTUAL_FLAG,
    [CHANNEL_DVPP_VENC] = PROF_VALID_PERIPHERAL_VIRTUAL_FLAG,
    [CHANNEL_DVPP_VDEC] = PROF_VALID_PERIPHERAL_VIRTUAL_FLAG,
    [CHANNEL_NPU_APP_MEM] = PROF_VALID_PERIPHERAL_VIRTUAL_FLAG,
    [CHANNEL_NPU_MEM] = PROF_VALID_PERIPHERAL_VIRTUAL_FLAG,
};

char *prof_peri_cb_regs[CHANNEL_IDS_MAX][PROF_PERI_SAMPLE_MODE_MAX] = {
    [CHANNEL_DDR][PROF_PERI_SAMPLE] = PROF_DDR_SAMPLE_FUNC_NAME,
    [CHANNEL_DDR][PROF_PERI_SAMPLE_USERDATA] = PROF_DDR_USERDATA_FUNC_NAME,
    [CHANNEL_LLC][PROF_PERI_SAMPLE] = PROF_LLC_SAMPLE_FUNC_NAME,
    [CHANNEL_LLC][PROF_PERI_SAMPLE_USERDATA] = PROF_LLC_USERDATA_FUNC_NAME,
    [CHANNEL_LLC][PROF_PERI_SAMPLE_STOP] = PROF_LLC_STOP_FUNC_NAME,

#ifdef CFG_FEATURE_RC_MODE
    [CHANNEL_NIC][PROF_PERI_SAMPLE] = PROF_NIC_SAMPLE_FUNC_NAME,
#endif
    [CHANNEL_DVPP][PROF_PERI_SAMPLE] = PROF_DVPP_SAMPLE_FUNC_NAME,
    [CHANNEL_DVPP_VENC][PROF_PERI_SAMPLE] = PROF_DVPP_VENC_SAMPLE_FUNC_NAME,
    [CHANNEL_DVPP_VDEC][PROF_PERI_SAMPLE] = PROF_DVPP_VDEC_SAMPLE_FUNC_NAME,
    [CHANNEL_NPU_APP_MEM][PROF_PERI_SAMPLE] = PROF_NPU_APP_SAMPLE_FUNC_NAME,
    [CHANNEL_NPU_APP_MEM][PROF_PERI_SAMPLE_USERDATA] = PROF_NPU_APP_USERDATA_FUNC_NAME,
    [CHANNEL_NPU_MEM][PROF_PERI_SAMPLE] = PROF_NPU_SAMPLE_FUNC_NAME,
    [CHANNEL_NPU_MEM][PROF_PERI_SAMPLE_USERDATA] = PROF_NPU_USERDATA_FUNC_NAME,
};

int* prof_get_hash(void)
{
    return prof_hash;
}

char *prof_get_peri_cb_regs_handler(u32 channel_id, u32 mode)
{
    return prof_peri_cb_regs[channel_id][mode];
}

u32 prof_get_buff_data_len(u32 device_id, u32 channel_id, u32 vfid, u32 sub_channel_id)
{
    struct prof_sub_channel_info *sub_channel_info =
        prof_get_sub_channel_info_from_index(device_id, vfid, channel_id, sub_channel_id);
    return (sub_channel_info->buf_len - BUFF_HEAD_SIZE);
}

int prof_tscpu_init_channel_info(struct prof_sub_channel_info *sub_channel_info)
{
    struct prof_data_head *data_head = NULL;

    sub_channel_info->buf_len = PROF_TS_BUFFER_LEN;
    if (sub_channel_info->vir_addr == NULL) {
        if (prof_tscpu_alloc_channel_memory(sub_channel_info) != PROF_OK) {
            prof_err("Failed to make TS channel allocate the channel memory."
                " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u)\n",
                sub_channel_info->device_id, sub_channel_info->vfid, sub_channel_info->channel_id,
                sub_channel_info->sub_channel_id);
            return PROF_ERROR;
        }
    }

    data_head = (struct prof_data_head *)sub_channel_info->vir_addr;
    data_head->write_ptr = 0;
    data_head->read_ptr = 0;
    data_head->buf_len = sub_channel_info->buf_len;

#ifdef __aarch64__
    flush_cache((unsigned char *)data_head, BUFF_HEAD_SIZE);
#endif

    return PROF_OK;
}

int prof_alloc_all_tscpu_common_bufs(u32 device_id)
{
    return PROF_OK;
}

void prof_free_all_tscpu_common_bufs(u32 device_id)
{
    return;
}

int prof_data_flush_support_check(u32 channel_id)
{
    return PROF_NOT_SUPPORT;
}

u64 prof_sq_init_get_buf_pa(struct prof_sub_channel_info *sub_channel_info)
{
    return sub_channel_info->phy_addr;
}

u32 prof_sq_init_get_buf_len(struct prof_sub_channel_info *sub_channel_info)
{
    return sub_channel_info->buf_len;
}

u32 prof_sq_init_get_buf_num(u32 channel_id, u32 vfid)
{
    return 1;
}

u64 prof_get_reserved_mem(u32 device_id, u32 channel_id, u32 sub_ch_id)
{
    return 0;
}

void prof_init_tsid(struct prof_sub_channel_info *sub_channel_info)
{
    sub_channel_info->ts_channel.tsid = 0;
}

u32 prof_get_mem_type(int channel_id)
{
    return MEM_MALLOC;
}

int prof_channel_need_sync_rw_ptr(struct prof_sub_channel_info *sub_channel_info)
{
    int ret;
    switch (sub_channel_info->channel_id) {
        case CHANNEL_STARS_SOC_PROFILE_BUFFER:
        case CHANNEL_STARS_SOC_LOG_BUFFER:
        case CHANNEL_FFTS_PROFILE_BUFFER_TASK:
            ret = PROF_OK;
            break;
        default:
            ret = PROF_ERROR;
            break;
    }
    return ret;
}

STATIC int prof_get_available_hw_vir_sub_channel_id(u32 device_id, u32 channel_id)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;
    struct prof_device_resource_info *dev_resource_info =
        prof_get_device_resource_info(device_id);
    u32 i, j;

    mutex_lock(&(dev_resource_info->mutex));
    for (i = 0; i < PROF_SUB_CHANNEL_NUM_MAX; i++) {
        sub_channel_info = prof_get_sub_resource_info(device_id, channel_id, i);
        if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_IDLE) {
            sub_channel_info->channel_state = (u32)PROF_CHANNEL_DISABLE;
            break;
        }
    }

    if (i == PROF_SUB_CHANNEL_NUM_MAX) {
        mutex_unlock(&(dev_resource_info->mutex));
        return PROF_NOT_ENOUGH_SUB_CHANNEL_RESOURCE;
    }

    for (j = 0; j < CHANNEL_IDS_MAX; j++) {
        if (prof_is_valid_virtual_tscpu_hw_channel(j)) {
            sub_channel_info = prof_get_sub_resource_info(device_id, j, i);
            sub_channel_info->channel_state = (u32)PROF_CHANNEL_DISABLE;
        }
    }

    mutex_unlock(&(dev_resource_info->mutex));
    return i;
}

STATIC int prof_free_available_hw_vir_sub_channel_id(u32 device_id, u32 sub_channel_id)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;
    struct prof_device_resource_info *dev_resource_info =
        prof_get_device_resource_info(device_id);
    u32 i;

    mutex_lock(&(dev_resource_info->mutex));
    for (i = 0; i < CHANNEL_IDS_MAX; i++) {
        if (prof_is_valid_virtual_tscpu_hw_channel(i)) {
            sub_channel_info = prof_get_sub_resource_info(device_id, i, sub_channel_id);
            sub_channel_info->channel_state = (u32)PROF_CHANNEL_IDLE;
        }
    }
    mutex_unlock(&(dev_resource_info->mutex));
    return i;
}

STATIC int prof_get_available_sub_channel_id(u32 device_id, u32 channel_id)
{
    struct prof_sub_channel_info *sub_channel_info = NULL;
    struct prof_channel_resource_info *channel_resource_info = NULL;
    int i;

    channel_resource_info = prof_get_channel_resource_info(device_id, channel_id);
    mutex_lock(&(channel_resource_info->mutex));

    for (i = 0; i < PROF_SUB_CHANNEL_NUM_MAX; i++) {
        sub_channel_info = prof_get_sub_resource_info(device_id, channel_id, (u32)i);
        if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_IDLE) {
            sub_channel_info->channel_state = (u32)PROF_CHANNEL_DISABLE;
            mutex_unlock(&(channel_resource_info->mutex));
            return i;
        }
    }

    mutex_unlock(&(channel_resource_info->mutex));
    return PROF_NOT_ENOUGH_SUB_CHANNEL_RESOURCE;
}

int prof_get_available_sub_channel(struct prof_ioctl_para *para, struct prof_proc_ctx *proc_ctx,
    u32 phy_device_id, u32 *sub_channel_id)
{
    struct prof_sub_channel_res *sub_channel_res = &proc_ctx->sub_channel_res;
    u32 device_id = para->device_id;
    u32 channel_id = para->channel_id;
    int ret;

    if (prof_is_valid_virtual_tscpu_hw_channel(channel_id)) {
        mutex_lock(&sub_channel_res->mutex);

        /* Different channels of the same process must use the same sub_channel_id. */
        if (sub_channel_res->count[device_id] > 0) {
            *sub_channel_id = (u32)sub_channel_res->sub_channel_id[device_id];
            sub_channel_res->count[device_id]++;
            mutex_unlock(&sub_channel_res->mutex);
            return PROF_OK;
        }

        ret = prof_get_available_hw_vir_sub_channel_id(phy_device_id, channel_id);
        if (ret < 0) {
            mutex_unlock(&sub_channel_res->mutex);
            return ret;
        }
        sub_channel_res->sub_channel_id[device_id] = (u8)ret;
        sub_channel_res->count[device_id]++;
        *sub_channel_id = (u32)ret;
        mutex_unlock(&sub_channel_res->mutex);
    } else {
        ret = prof_get_available_sub_channel_id(phy_device_id, channel_id);
        if (ret < 0) {
            return ret;
        }
        *sub_channel_id = (u32)ret;
    }

    return PROF_OK;
}

void prof_sub_resource_channel_state_init(struct prof_sub_channel_info *sub_resource_info)
{
    sub_resource_info->channel_state = (u32)PROF_CHANNEL_UNINIT;

    if (prof_is_valid_virtual_tscpu_channel(sub_resource_info->channel_id) ||
        prof_is_valid_virtual_peri_channel(sub_resource_info->channel_id) ||
        (prof_is_valid_channel(sub_resource_info->channel_id) && sub_resource_info->sub_channel_id == 0)) {
        sub_resource_info->channel_state = PROF_CHANNEL_IDLE;
    }
}

void prof_free_sub_channel_id(struct prof_sub_channel_res *sub_channel_res,
    u32 device_id, u32 phy_devid, u32 channel_id, u32 sub_channel_id)
{
    struct prof_channel_resource_info *channel_resource_info =
        prof_get_channel_resource_info(phy_devid, channel_id);

    if (prof_is_valid_virtual_tscpu_hw_channel(channel_id) != true) {
        mutex_lock(&(channel_resource_info->mutex));
        channel_resource_info->sub_resource_info[sub_channel_id].channel_state = (u32)PROF_CHANNEL_IDLE;
        mutex_unlock(&(channel_resource_info->mutex));
        return;
    }

    mutex_lock(&sub_channel_res->mutex);
    sub_channel_res->count[device_id]--;
    if (sub_channel_res->count[device_id] == 0) {
        prof_free_available_hw_vir_sub_channel_id(phy_devid, sub_channel_id);
    }
    mutex_unlock(&sub_channel_res->mutex);
    prof_debug("Free sub channel id. (device_id=%u; phy_devid=%u; channel_id=%u; sub_channel_id=%u)",
        device_id, phy_devid, channel_id, sub_channel_id);
}

int prof_ctx_sub_channel_res_init(struct prof_sub_channel_res *sub_channel_res)
{
    mutex_init(&sub_channel_res->mutex);
    return PROF_OK;
}

void prof_ctx_sub_channel_res_uninit(struct prof_sub_channel_res *sub_channel_res)
{
    mutex_destroy(&sub_channel_res->mutex);
}

int prof_tscpu_platform_read_sub_ch(struct prof_ioctl_para *para,
    struct prof_sub_channel_info *sub_channel_info)
{
    int ret;

    ret = prof_tscpu_ringbuffer(sub_channel_info->vir_addr, para,
        &sub_channel_info->prof_dfx, sub_channel_info->sub_channel_id);
    if (ret != PROF_OK) {
        prof_err("Failed to read ringbuffer."
            " (devid=%u; vfid=%u; channel_id=%u; ret=%d)\n",
            sub_channel_info->device_id, sub_channel_info->vfid, sub_channel_info->channel_id, ret);
    }

    return ret;
}

u32 prof_tscpu_get_data_len(struct prof_sub_channel_info *sub_channel_info)
{
    return prof_tscpu_get_one_zone_data_len((unsigned char *)sub_channel_info->vir_addr,
        prof_get_buff_data_len(sub_channel_info->device_id, sub_channel_info->channel_id,
        sub_channel_info->vfid, sub_channel_info->sub_channel_id));
}

void prof_platform_ts_buff_free_flag(enum prof_ts_buff_free_flag *flag)
{
    *flag = TS_BUFF_NOT_FREE;
}

void prof_platform_ts_buff_not_free_op(struct prof_sub_channel_info *sub_channel_info)
{
}

void prof_init_hash(void)
{
    return;
}
#else
void prof_ut_test_ascd310B(void)
{
}
module_init(prof_ut_test_ascd310B);
#endif
