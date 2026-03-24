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

#include "bbox_message.h"
#include <linux/securec.h>
#include "bbox_communication.h"
#include "common/bbox_print.h"
#include "common/bbox_common.h"
#ifdef CFG_FEATURE_MDC_CLOCK_CONFIG
#include <linux/virt_wall_time.h>
#include "bootparam/bbox_boot_param.h"
#endif

STATIC struct send_info g_seq_map[MAX_TIMESTAMP_SEQ] = {0};    // send info for time sequence
STATIC DEFINE_SPINLOCK(g_seq_map_lock);

/*
 * @brief       : send reboot type msg to host
 * @param [in]  : u32 devid             device id
 * @param [in]  : u8 reboot_type        reboot type
 * @param [in]  : u64 addr              devmem addr
 * @param [in]  : u32 len               devmem len
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_submit_reboot_message(u32 devid, u8 reboot_type, u64 addr, u32 len)
{
    struct bbox_msg_node *node = NULL;
    struct bbox_reboot_msg *msg = NULL;

    node = (struct bbox_msg_node *)bbox_vmalloc(sizeof(struct bbox_msg_node));
    if (node == NULL) {
        BB_PRINT_ERR("bbox vmalloc restart node failed.\n");
        return BBOX_FAILURE;
    }

    node->node_type = MSG_NODE_TYPE_DYNAMIC;
    node->devid = devid;
    node->notify = NULL;
    node->arg = NULL;
    node->msg = (struct bbox_msg_header *)bbox_vmalloc(sizeof(struct bbox_reboot_msg));
    if (node->msg == NULL) {
        BB_PRINT_ERR("bbox vmalloc restart msg failed.\n");
        bbox_vfree(node);
        return BBOX_FAILURE;
    }

    msg = (struct bbox_reboot_msg *)node->msg;
    msg->header.type = BBOX_MSG_REBOOT;
    msg->header.len = (u32)sizeof(struct bbox_reboot_msg);
    msg->reboot_type = reboot_type;
    msg->devmem_len = len;
    msg->devmem_addr = addr;

    BB_PRINT_INFO("[device-%u] notify start reason: 0x%hhx.\n", devid, reboot_type);
    bbox_submit_message(node);

    return BBOX_SUCCESS;
}

/*
 * @brief       : save data to hdc node, use to send
 * @param [in]  : const struct rdr_exception_msg_info *info     hdc send need info
 * @param [in]  : const char *data                          send date
 * @param [in]  : u32 data_len                              data length
 * @param [in]  : u32 data_type                             data type
 * @return      : hdc buffer addr
 */
STATIC struct bbox_exception_msg *bbox_set_exception_message(const struct rdr_exception_msg_info *info,
                                                             const char *data, u32 data_len, u32 data_type)
{
    s32 ret;
    u32 length;
    void *addr = NULL;
    struct bbox_exception_msg *msg = NULL;

    BB_CHECK_PTR(data, return NULL, "invalid param, data is NULL.\n");
    BB_CHECK_PTR(info, return NULL, "invalid param, info is NULL.\n");
    BB_CHECK_EXP_ACT((data_len == 0), return NULL, "invalid param, length:%u.\n", data_len);
    BB_CHECK_EXP_ACT((data_len >= (BBOX_UINT_INVALID - sizeof(struct bbox_exception_msg))),
        return NULL, "invalid param, length:%u.\n", data_len);

    length = data_len + (u32)sizeof(struct bbox_exception_msg);
    addr = bbox_vmalloc(length);
    if (addr == NULL) {
        BB_PRINT_ERR("malloc buffer failed. excepid[0x%x], coreid[%d], len[%u], type[%u].\n",
                     info->excepid, info->coreid, data_len, data_type);
        return NULL;
    }

    msg = (struct bbox_exception_msg *)addr;
    msg->header.type = BBOX_MSG_EXCEPTION;
    msg->header.len = length;
    msg->excepid = info->excepid;
    msg->coreid = info->coreid;
    msg->tm.tv_sec = info->tm.tv_sec;
    msg->tm.tv_nsec = info->tm.tv_nsec;
    msg->dlen = data_len;
    msg->dtype = data_type;
    msg->etype = info->etype;

#if (defined BBOX_SOC_PLATFORM_MDC) && (!defined BBOX_UT)
    ret = bbox_copy_exception_msg((void *)msg->data, data, data_len);
#else
    ret = memcpy_s((void *)msg->data, data_len, data, data_len);
#endif
    if (ret != EOK) {
        BB_PRINT_ERR("[%s][%4d] memcpy_s failed.\n", __func__, __LINE__);
        bbox_vfree(addr);
        return NULL;
    }
    return msg;
}

/*
 * @brief       : save module data to hdc node, use to send
 * @param [in]  : const rdr_exception_msg_info *info        send need info
 * @param [in]  : const char *data                          send date
 * @param [in]  : u32 data_len                              data length
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_submit_exception_message(const struct rdr_exception_msg_info *info,
                                         const char *data, u32 len, u32 data_type)
{
    struct bbox_msg_node *node = NULL;

    BB_CHECK_PTR(info, return BBOX_FAILURE, "invalid parameter, info is NULL.\n");
    BB_CHECK_PTR(data, return BBOX_FAILURE, "invalid parameter, data is NULL.\n");

    node = (struct bbox_msg_node *)bbox_vmalloc(sizeof(struct bbox_msg_node));
    if (node == NULL) {
        BB_PRINT_ERR("bbox vmalloc exception node failed.\n");
        return BBOX_FAILURE;
    }

    node->node_type = MSG_NODE_TYPE_DYNAMIC;
    node->coreid = info->coreid;
    node->excepid = info->excepid;
    node->devid = info->devid;
    node->notify = NULL;
    node->arg = NULL;

    node->msg = (struct bbox_msg_header *)bbox_set_exception_message(info, data, len, data_type);
    if (node->msg == NULL) {
        bbox_vfree(node);
        return BBOX_FAILURE;
    }

    bbox_submit_message(node);
    return BBOX_SUCCESS;
}

#ifdef CFG_FEATURE_MDC_CLOCK_CONFIG
STATIC s32 bbox_submit_exception_message_vtime(const struct rdr_exception_msg_info *info, const char *data,
    u32 data_len, u32 data_type)
{
    struct rdr_exception_msg_info tmpInfo = { 0 };

    tmpInfo.coreid = info->coreid;
    tmpInfo.devid = info->devid;
    tmpInfo.etype = info->etype;
    tmpInfo.excepid = info->excepid;
    tmpInfo.tm.tv_sec = info->tv.tv_sec;
    tmpInfo.tm.tv_nsec = info->tv.tv_nsec;

    BB_PRINT_INFO("time updates.(tm=%llu.%llu, tv=%llu.%llu)\n",
        info->tm.tv_sec, info->tm.tv_nsec, info->tv.tv_sec, info->tv.tv_nsec);
    return bbox_submit_exception_message(&tmpInfo, data, data_len, data_type);
}
#endif

/*
 * @brief       : save pmu registers data to node, use to send
 * @param [in]  : const struct rdr_exception_msg_info *info     hdc send need info
 * @param [in]  : const char *data                              send date
 * @param [in]  : u32 data_len                                  data length
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_submit_pmu_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len)
{
    return bbox_submit_exception_message(info, data, data_len, BBOX_DATASET_PMU);
}

/*
 * @brief       : save tsensor registers data to node, use to send
 * @param [in]  : const struct rdr_exception_msg_info *info     hdc send need info
 * @param [in]  : const char *data                              send date
 * @param [in]  : u32 data_len                                  data length
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_submit_tsensor_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len)
{
    return bbox_submit_exception_message(info, data, data_len, BBOX_DATASET_TSENSOR);
}

/*
 * @brief       : save sram data to node, use to send
 * @param [in]  : const struct rdr_exception_msg_info *info     hdc send need info
 * @param [in]  : const char *data                              send date
 * @param [in]  : u32 data_len                                  data length
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_submit_sram_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len)
{
    return bbox_submit_exception_message(info, data, data_len, BBOX_DATASET_SRAM);
}

/*
 * @brief       : save kernel log data to node, use to send
 * @param [in]  : const struct rdr_exception_msg_info *info     hdc send need info
 * @param [in]  : const char *data                              send date
 * @param [in]  : u32 data_len                                  data length
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_submit_klog_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len)
{
    return bbox_submit_exception_message(info, data, data_len, BBOX_DATASET_KLOG);
}

/*
 * @brief       : save log data to node, use to send
 * @param [in]  : const struct rdr_exception_msg_info *info     hdc send need info
 * @param [in]  : const char *data                              send date
 * @param [in]  : u32 data_len                                  data length
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_submit_log_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len)
{
#ifdef CFG_FEATURE_MDC_CLOCK_CONFIG
    return bbox_submit_exception_message_vtime(info, data, data_len, BBOX_DATASET_LOG);
#else
    return bbox_submit_exception_message(info, data, data_len, BBOX_DATASET_LOG);
#endif
}

/*
 * @brief       : save device-x bbox ddr data to node, use to send
 * @param [in]  : const struct rdr_exception_msg_info *info     hdc send need info
 * @param [in]  : const char *data                              send date
 * @param [in]  : u32 data_len                                  data length
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_submit_ddr_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len)
{
    if ((data_len == 0) || (data == NULL)) {
        const char *chart = "NO DDR DATA.";
        return bbox_submit_exception_message(info, chart, (u32)strlen(chart), BBOX_DATASET_NODDR);
    } else {
        return bbox_submit_exception_message(info, data, data_len, BBOX_DATASET_DDR);
    }
}

/*
 * @brief       : save module data to node, use to send
 * @param [in]  : const struct rdr_exception_msg_info *info     hdc send need info
 * @param [in]  : const char *data                              send date
 * @param [in]  : u32 data_len                                  data length
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_submit_module_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len)
{
#ifdef CFG_FEATURE_MDC_CLOCK_CONFIG
    return bbox_submit_exception_message_vtime(info, data, data_len, BBOX_DATASET_FULL);
#else
    return bbox_submit_exception_message(info, data, data_len, BBOX_DATASET_FULL);
#endif
}

/*
 * @brief       : save module recover data to node, use to send
 * @param [in]  : const struct rdr_exception_msg_info *info     hdc send need info
 * @param [in]  : const char *data                              send date
 * @param [in]  : u32 data_len                                  data length
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_submit_module_reset_message(const struct rdr_exception_msg_info *info, const char *data, u32 data_len)
{
    return bbox_submit_exception_message(info, data, data_len, BBOX_DATASET_RECOVER);
}

/*
 * @brief       : submit message to remote, if success, notify callback
 * @param [in]  : struct rdr_exception_msg_info *info       send info
 * @param [in]  : node_free_notify_ptr notify               notify function
 * @param [in]  : char *data                data
 * @param [in]  : u32 len                   data length
 * @param [in]  : u32 data_type             data type
 * @return      : !=0 failure; >=0 success
 */
STATIC s32 bbox_submit_notify_message(const struct rdr_exception_msg_info *info,
                                      const node_free_notify_ptr notify,
                                      const char *data, u32 len, u32 data_type)
{
    u32 *devid = NULL;
    struct bbox_msg_node *node = NULL;

    BB_CHECK_PTR(info, return BBOX_FAILURE, "invalid parameter, info is NULL.\n");
    BB_CHECK_PTR(data, return BBOX_FAILURE, "invalid parameter, data is NULL.\n");
    BB_CHECK_PTR(notify, return BBOX_FAILURE, "invalid parameter, notify is NULL.\n");
    BB_CHECK_EXP_ACT((len == 0), return BBOX_FAILURE, "invalid parameter, len is %u.\n", len);

    node = (struct bbox_msg_node *)bbox_vmalloc(sizeof(struct bbox_msg_node));
    if (node == NULL) {
        BB_PRINT_PN("bbox vmalloc hdr node failed\n");
        return BBOX_FAILURE;
    }

    devid = (u32 *)bbox_vmalloc(sizeof(u32));
    if (devid == NULL) {
        BB_PRINT_PN("bbox vmalloc devid failed\n");
        bbox_vfree(node);
        return BBOX_FAILURE;
    }
    *devid = info->devid;

    node->coreid = info->coreid;
    node->excepid = info->excepid;
    node->devid = info->devid;
    node->node_type = MSG_NODE_TYPE_DYNAMIC;
    node->notify = notify;
    node->arg = (void *)devid;

    node->msg = (struct bbox_msg_header *)bbox_set_exception_message(info, data, len, data_type);
    if (node->msg == NULL) {
        bbox_vfree(node);
        bbox_vfree(devid);
        return BBOX_FAILURE;
    }

    bbox_submit_message(node);
    return BBOX_SUCCESS;
}

/*
 * @brief       : submit hdr message to remote
 * @param [in]  : struct rdr_exception_msg_info *info      send info
 * @param [in]  : char *data                data
 * @param [in]  : u32 len                   data length
 * @return      : !=0 failure; >=0 success
 */
s32 bbox_submit_hdr_message(const struct rdr_exception_msg_info *info,
                            const node_free_notify_ptr notify,
                            const char *data,
                            u32 len)
{
    return bbox_submit_notify_message(info, notify, data, len, BBOX_DATASET_HDR);
}

/*
 * @brief       : submit cdr message to remote
 * @param [in]  : struct rdr_exception_msg_info *info      send info
 * @param [in]  : char *data                data
 * @param [in]  : u32 len                   data length
 * @return      : !=0 failure; >=0 success
 */
s32 bbox_submit_cdr_message(const struct rdr_exception_msg_info *info,
                            const node_free_notify_ptr notify,
                            const char *data,
                            u32 len)
{
    return bbox_submit_notify_message(info, notify, data, len, BBOX_DATASET_CDR);
}

/*
 * @brief           : find send info correspond to time stamp
 * @param [in]      : u64 t_seq                         time sequence
 * @param [in]      : struct bbox_time tm               time stamp
 * @return          : TRUE/FALSE
 */
STATIC s32 bbox_find_send_info(u64 t_seq, struct bbox_time tm)
{
    if (g_seq_map[t_seq].send_seq == 0) {
        return BBOX_FALSE;
    }
    if (g_seq_map[t_seq].tm.tv_sec != tm.tv_sec) {
        return BBOX_FALSE;
    }
    if (g_seq_map[t_seq].tm.tv_usec != (tm.tv_nsec / KILO)) {
        return BBOX_FALSE;
    }
    return BBOX_TRUE;
}

/*
 * @brief           : use to solve the same time stamp exception after device reboot
 * @param [in]      : struct channel_info *channel      channel info
 * @param [in]      : struct bbox_msg_header *hdr       msg header
 * @param [in/out]  : u64 *tv_nsec                      original value
 * @return          : NA
 */
void bbox_message_set_time_seq(struct channel_info *channel, struct bbox_msg_header *hdr, u64 *tv_nsec)
{
    u64 t_seq;
    lock_flag_t flags = 0;
    struct bbox_exception_msg *msg = NULL;

    BB_CHECK_PTR(channel, return, "channel is NULL.\n");
    BB_CHECK_PTR(hdr, return, "hdr is NULL.\n");
    BB_CHECK_PTR(tv_nsec, return, "tv_nsec is NULL.\n");

    if ((hdr->type != (u32)BBOX_MSG_OOM) && (hdr->type != (u32)BBOX_MSG_EXCEPTION)) {
        return;
    }

    msg = (struct bbox_exception_msg *)hdr;
    if (*tv_nsec != 0) {
        // set msg->tm.tv_nsec back to its original value
        BB_PRINT_INFO("set timestamp %llu back to %llu.\n", msg->tm.tv_nsec, *tv_nsec);
        msg->tm.tv_nsec = *tv_nsec;
        return;
    }
    // record original value
    *tv_nsec = msg->tm.tv_nsec;

    t_seq = msg->tm.tv_nsec % KILO;
    msg->tm.tv_nsec -= t_seq;
    spin_lock_irqsave(&g_seq_map_lock, flags);
    if (bbox_find_send_info(t_seq, msg->tm) == BBOX_TRUE) {
        msg->tm.tv_nsec += g_seq_map[t_seq].send_seq;
        spin_unlock_irqrestore(&g_seq_map_lock, flags);
        BB_PRINT_INFO("find t_seq in map and update timestamp from %llu to %llu.\n", *tv_nsec, msg->tm.tv_nsec);
        return;
    }
    // add send info to g_seq_map
    g_seq_map[t_seq].send_seq =  channel->send_seq;
    g_seq_map[t_seq].tm.tv_sec = msg->tm.tv_sec;
    g_seq_map[t_seq].tm.tv_usec = msg->tm.tv_nsec / KILO;
    spin_unlock_irqrestore(&g_seq_map_lock, flags);
    msg->tm.tv_nsec += channel->send_seq;
    BB_PRINT_INFO("add seq map from %llu to %u.\n", t_seq, channel->send_seq);
    // 0 reserved for host, the range of device seq if [1, KILO)
    channel->send_seq = (channel->send_seq % (KILO - 1U)) + 1U;
    BB_PRINT_INFO("update timestamp from %llu to %llu.\n", *tv_nsec, msg->tm.tv_nsec);
}
