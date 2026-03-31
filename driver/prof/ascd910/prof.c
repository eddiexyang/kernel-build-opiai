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

#include "dbl/chip_config.h"

#include "prof.h"
#include "prof_drv_dev.h"

int prof_hash[CHANNEL_IDS_MAX] = {
    [CHANNEL_HBM] = PROF_VALID_PERIPHERAL_FLAG,
    [CHANNEL_PCIE] = PROF_VALID_PERIPHERAL_FLAG,
    [CHANNEL_NIC] = PROF_VALID_PERIPHERAL_FLAG,
    [CHANNEL_DVPP] = PROF_VALID_PERIPHERAL_FLAG,
    [CHANNEL_DDR] = PROF_VALID_PERIPHERAL_FLAG,
    [CHANNEL_LLC] = PROF_VALID_PERIPHERAL_FLAG,
    [CHANNEL_HCCS] = PROF_VALID_PERIPHERAL_FLAG,

    [CHANNEL_TSCPU] = PROF_VALID_TSCPU_FLAG,
    [CHANNEL_AICORE] = PROF_VALID_TSCPU_HW_VIRTUAL_FLAG,
    [CHANNEL_TSFW] = PROF_VALID_TSCPU_SW_VIRTUAL_FLAG,

    [CHANNEL_HWTS_LOG] = PROF_VALID_TSCPU_HW_VIRTUAL_FLAG,
    [CHANNEL_KEY_POINT] = PROF_VALID_TSCPU_SW_VIRTUAL_FLAG,
    [CHANNEL_TSFW_L2] = PROF_VALID_TSCPU_FLAG,
    [CHANNEL_ROCE] = PROF_VALID_PERIPHERAL_FLAG,
    [CHANNEL_NPU_APP_MEM] = PROF_VALID_PERIPHERAL_VIRTUAL_FLAG,
    [CHANNEL_NPU_MEM] = PROF_VALID_PERIPHERAL_VIRTUAL_FLAG,
};

int prof_tscpu_buf_len_hash[CHANNEL_IDS_MAX] = {
    [CHANNEL_AICORE] = PROF_TS_BUFFER_LEN_MAX,
    [CHANNEL_HWTS_LOG] = PROF_TS_BUFFER_LEN_MAX,
};

char *prof_peri_cb_regs[CHANNEL_IDS_MAX][PROF_PERI_SAMPLE_MODE_MAX] = {
    [CHANNEL_HBM][PROF_PERI_SAMPLE] = PROF_HBM_SAMPLE_FUNC_NAME,
    [CHANNEL_HBM][PROF_PERI_SAMPLE_USERDATA] = PROF_HBM_USERDATA_FUNC_NAME,
    [CHANNEL_PCIE][PROF_PERI_SAMPLE] = PROF_PCIE_SAMPLE_FUNC_NAME,
    [CHANNEL_PCIE][PROF_PERI_SAMPLE_USERDATA] = PROF_PCIE_USERDATA_FUNC_NAME,
    [CHANNEL_PCIE][PROF_PERI_SAMPLE_STOP] = PROF_PCIE_STOP_FUNC_NAME,
    [CHANNEL_NIC][PROF_PERI_SAMPLE] = PROF_NIC_SAMPLE_FUNC_NAME, // matebook don't load gmac ko, so need to lookup;
    [CHANNEL_DVPP][PROF_PERI_SAMPLE] = PROF_DVPP_SAMPLE_FUNC_NAME,
    [CHANNEL_DDR][PROF_PERI_SAMPLE] = PROF_DDR_SAMPLE_FUNC_NAME,
    [CHANNEL_DDR][PROF_PERI_SAMPLE_USERDATA] = PROF_DDR_USERDATA_FUNC_NAME,
    [CHANNEL_LLC][PROF_PERI_SAMPLE] = PROF_LLC_SAMPLE_FUNC_NAME,
    [CHANNEL_LLC][PROF_PERI_SAMPLE_USERDATA] = PROF_LLC_USERDATA_FUNC_NAME,
    [CHANNEL_LLC][PROF_PERI_SAMPLE_STOP] = PROF_LLC_STOP_FUNC_NAME,
    [CHANNEL_HCCS][PROF_PERI_SAMPLE] = PROF_HCCS_SAMPLE_FUNC_NAME,
    [CHANNEL_HCCS][PROF_PERI_SAMPLE_STOP] = PROF_HCCS_STOP_FUNC_NAME,
    [CHANNEL_ROCE][PROF_PERI_SAMPLE] = PROF_ROCE_SAMPLE_FUNC_NAME,
    [CHANNEL_NPU_APP_MEM][PROF_PERI_SAMPLE] = PROF_NPU_APP_SAMPLE_FUNC_NAME,
    [CHANNEL_NPU_APP_MEM][PROF_PERI_SAMPLE_USERDATA] = PROF_NPU_APP_USERDATA_FUNC_NAME,
    [CHANNEL_NPU_MEM][PROF_PERI_SAMPLE] = PROF_NPU_SAMPLE_FUNC_NAME,
    [CHANNEL_NPU_MEM][PROF_PERI_SAMPLE_USERDATA] = PROF_NPU_USERDATA_FUNC_NAME,
};

int* prof_get_hash(void)
{
    return prof_hash;
}

STATIC u32 prof_get_com_buf_len(u32 channel_id)
{
    return (u32)prof_tscpu_buf_len_hash[channel_id];
}

char *prof_get_peri_cb_regs_handler(u32 channel_id, u32 mode)
{
    return prof_peri_cb_regs[channel_id][mode];
}

u32 prof_tscpu_start_get_buf_len(u32 channel_id)
{
    if (channel_id == CHANNEL_HWTS_LOG) {
        prof_info("The current [buf_len] was over four times of its original value."
            " (channel_id=%u; buf_len=%d)\n", channel_id, PROF_TS_BUFFER_LEN_MAX);
        return PROF_TS_BUFFER_LEN_MAX;
    }
    return PROF_TS_BUFFER_LEN;
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

int prof_data_flush_support_check(u32 channel_id)
{
    if (channel_id == CHANNEL_HWTS_LOG || channel_id == CHANNEL_TSFW) {
        return PROF_OK;
    }

    return PROF_NOT_SUPPORT;
}

u32 prof_get_buff_data_len(u32 device_id, u32 channel_id, u32 vfid, u32 sub_channel_id)
{
    struct prof_sub_channel_info *sub_channel_info =
        prof_get_sub_channel_info_from_index(device_id, vfid, channel_id, sub_channel_id);
    return (sub_channel_info->buf_len - BUFF_HEAD_SIZE);
}

u32 prof_get_mem_type(int channel_id)
{
    return MEM_MALLOC;
}

void prof_init_tsid(struct prof_sub_channel_info *sub_channel_info)
{
    sub_channel_info->ts_channel.tsid = 0;
}

u64 prof_get_reserved_mem(u32 device_id, u32 channel_id, u32 sub_ch_id)
{
    return 0;
}

int prof_channel_need_sync_rw_ptr(struct prof_sub_channel_info *sub_channel_info)
{
    if (sub_channel_info->channel_id == CHANNEL_HWTS_LOG) {
        return PROF_OK;
    } else {
        return PROF_ERROR;
    }
}

int prof_tscpu_init_channel_info(struct prof_sub_channel_info *sub_channel_info)
{
    struct prof_data_head *data_head = NULL;
    int ret;

    sub_channel_info->buf_len = prof_tscpu_start_get_buf_len(sub_channel_info->channel_id);
    if (sub_channel_info->vir_addr == NULL) {
        ret = prof_tscpu_alloc_channel_memory(sub_channel_info);
        if (ret != PROF_OK) {
            prof_err("Failed to allocate the channel memory for TS channel."
                " (device_id=%u; vfid=%u; channel_id=%u; sub_channel_id=%u; ret=%d)\n",
                sub_channel_info->device_id, sub_channel_info->vfid, sub_channel_info->channel_id,
                sub_channel_info->sub_channel_id, ret);
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

int prof_alloc_tscpu_common_buf(u32 device_id, u32 channel_id)
{
    prof_tscpu_buf_info_t *common_buf_info = NULL;
    struct prof_data_head *data_head = NULL;
    struct page *page = NULL;

    /* software channels support data split by vf in computing virtualization scene */
    if (prof_is_valid_virtual_tscpu_hw_channel(channel_id) == false) {
        return PROF_OK;
    }

    common_buf_info = prof_tscpu_get_common_buf_info(device_id, channel_id);
    page = alloc_pages_node(dbl_get_ts_default_nid(device_id), __GFP_ZERO | __GFP_THISNODE | GFP_HIGHUSER_MOVABLE,
        (u32)get_order(prof_get_com_buf_len(channel_id)));
    if (page == NULL) {
        prof_err("Failed to allocate the channel buffer for TS channel."
            "(buf_len=%u)\n", prof_get_com_buf_len(channel_id));
        return PROF_ERROR;
    }

    common_buf_info->virt_addr = (unsigned char *)page_to_virt(page);
    common_buf_info->phy_addr = virt_to_phys(common_buf_info->virt_addr);
    common_buf_info->len = prof_get_com_buf_len(channel_id);

    /* just adapt for TS */
    data_head = (struct prof_data_head *)common_buf_info->virt_addr;
    data_head->write_ptr = 0;
    data_head->read_ptr = 0;
    data_head->buf_len = prof_get_com_buf_len(channel_id);
    prof_event("Profile allocated the buffer for TS channel successfully."
        "(device_id=%u; channel_id=%u)\n", device_id, channel_id);

    return PROF_OK;
}

void prof_free_tscpu_common_buf(u32 device_id, u32 channel_id)
{
    prof_tscpu_buf_info_t *common_buf_info = NULL;

    if (prof_is_valid_virtual_tscpu_hw_channel(channel_id) == false) {
        return;
    }

    common_buf_info = prof_tscpu_get_common_buf_info(device_id, channel_id);
    if (common_buf_info->virt_addr != NULL) {
        free_pages((unsigned long)common_buf_info->virt_addr, (u32)get_order(prof_get_com_buf_len(channel_id)));
        common_buf_info->virt_addr = NULL;
        common_buf_info->phy_addr = 0;
        common_buf_info->len = 0;

        prof_event("The common buffer of TS channel was free successfully."
            "(device_id=%u; channel_id=%u)\n", device_id, channel_id);
    }
}

int prof_alloc_all_tscpu_common_bufs(u32 device_id)
{
    int i, j;
    int ret;

    for (i = 0; i < PROF_CHANNEL_NUM; i++) {
        ret = prof_alloc_tscpu_common_buf(device_id, (u32)i);
        if (ret != PROF_OK) {
            for (j = 0; j < i; j++) {
                prof_free_tscpu_common_buf(device_id, (u32)j);
            }
            return PROF_ERROR;
        }
    }

    return PROF_OK;
}

void prof_free_all_tscpu_common_bufs(u32 device_id)
{
    int i;

    for (i = 0; i < PROF_CHANNEL_NUM; i++) {
        prof_free_tscpu_common_buf(device_id, (u32)i);
    }
}

void prof_sub_resource_channel_state_init(struct prof_sub_channel_info *sub_resource_info)
{
    sub_resource_info->channel_state = (u32)PROF_CHANNEL_UNINIT;

    if (prof_is_valid_virtual_tscpu_channel(sub_resource_info->channel_id) ||
        prof_is_valid_virtual_peri_channel(sub_resource_info->channel_id) ||
        (prof_is_valid_channel(sub_resource_info->channel_id) && sub_resource_info->sub_channel_id == 0)) {
        sub_resource_info->channel_state = (u32)PROF_CHANNEL_IDLE;
    }
}

int prof_get_available_sub_channel(struct prof_ioctl_para *para, struct prof_proc_ctx *proc_ctx,
    u32 phy_device_id, u32 *sub_channel_id)
{
    struct prof_channel_resource_info *channel_resource_info = NULL;
    struct prof_sub_channel_info *sub_channel_info = NULL;
    u32 device_id = para->device_id;
    u32 vfid = para->vfid;
    u32 channel_id = para->channel_id;

    channel_resource_info = prof_get_channel_resource_info(device_id, channel_id);
    mutex_lock(&(channel_resource_info->mutex));

    sub_channel_info = prof_get_sub_resource_info(device_id, channel_id, vfid);
    if (sub_channel_info->channel_state == (u32)PROF_CHANNEL_IDLE) {
        *sub_channel_id = vfid;
        sub_channel_info->channel_state = (u32)PROF_CHANNEL_DISABLE;
        mutex_unlock(&(channel_resource_info->mutex));
        return PROF_OK;
    }

    mutex_unlock(&(channel_resource_info->mutex));
    return PROF_NOT_ENOUGH_SUB_CHANNEL_RESOURCE;
}

void prof_free_sub_channel_id(struct prof_sub_channel_res *sub_channel_res,
    u32 device_id, u32 phy_devid, u32 channel_id, u32 sub_channel_id)
{
    struct prof_channel_resource_info *channel_resource_info =
        prof_get_channel_resource_info(device_id, channel_id);
    mutex_lock(&(channel_resource_info->mutex));
    channel_resource_info->sub_resource_info[sub_channel_id].channel_state = (u32)PROF_CHANNEL_IDLE;
    mutex_unlock(&(channel_resource_info->mutex));
}

int prof_ctx_sub_channel_res_init(struct prof_sub_channel_res *sub_channel_res)
{
    return PROF_OK;
}

void prof_ctx_sub_channel_res_uninit(struct prof_sub_channel_res *sub_channel_res)
{
    return;
}

int prof_tscpu_platform_read_sub_ch(struct prof_ioctl_para *para,
    struct prof_sub_channel_info *sub_channel_info)
{
    int ret;

    ret = prof_tscpu_ringbuffer(sub_channel_info->vir_addr, para,
        &sub_channel_info->prof_dfx, sub_channel_info->sub_channel_id);
    if (ret != PROF_OK) {
        prof_err("Failed to read ringbuffer."
            " (devid=%d; vfid=%u; channel_id=%u; ret=%d)\n",
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
}

void prof_platform_ts_buff_not_free_op(struct prof_sub_channel_info *sub_channel_info)
{
}

void prof_init_hash(void)
{
    return;
}
#else
void prof_ut_test_ascd910(void)
{
}
module_init(prof_ut_test_ascd910);
#endif
