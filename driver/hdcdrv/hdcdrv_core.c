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

#include <linux/errno.h>
#include <linux/version.h>
#include <linux/jiffies.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/random.h>
#include <linux/jiffies.h>
#include <linux/vmalloc.h>
#include <linux/kallsyms.h>
#include <linux/kfifo.h>

#include "kernel_version_adapt.h"
#include "hdcdrv_sysfs.h"
#include "hdcdrv_interface.h"
#include "davinci_interface.h"
#include "hdcdrv_core.h"
#ifdef CFG_FEATURE_HDC_REG_MEM
#include "hdcdrv_status.h"
#endif
struct hdcdrv_ctrl *hdc_ctrl = NULL;
struct hdcdrv_node_tree_ctrl *hdc_node_tree = NULL;

struct hdcdrv_session_notify g_session_notify[HDCDRV_SUPPORT_MAX_SERVICE];

u32 hdcdrv_dev_num = 0;

report_process_status hdcdrv_report_process_status_func = NULL;
bandwidth_limit_check hdcdrv_bandwidth_limit_check_func = NULL;

struct vm_operations_struct hdcdrv_vm_ops_managed;

STATIC int g_rx_notify_sched_mode = HDCDRV_RX_SCHED_TASKLET;

struct hdcdrv_register_symbol g_hdcdrv_register_symbol = {.module_ptr = NULL, .wake_up_context_status = NULL};
rwlock_t g_symbol_lock;

static struct {
    char str[HDCDRV_STR_NAME_LEN];
} hdcdrv_service_type_str[HDCDRV_SUPPORT_MAX_SERVICE + 1] = {
    { "service_dmp" },                      /* HDCDRV_SERVICE_TYPE_DMP */
    { "service_profiling" },                /* HDCDRV_SERVICE_TYPE_PROFILING */
    { "service_IDE1" },                     /* HDCDRV_SERVICE_TYPE_IDE1 */
    { "service_file_trans" },               /* HDCDRV_SERVICE_TYPE_FILE_TRANS */
    { "service_IDE2" },                     /* HDCDRV_SERVICE_TYPE_IDE2 */
    { "service_log" },                      /* HDCDRV_SERVICE_TYPE_LOG */
    { "service_rdma" },                     /* HDCDRV_SERVICE_TYPE_RDMA */
    { "service_bbox" },                     /* HDCDRV_SERVICE_TYPE_BBOX */
    { "service_framework" },                /* HDCDRV_SERVICE_TYPE_FRAMEWORK */
    { "service_TSD" },                      /* HDCDRV_SERVICE_TYPE_TSD */
    { "service_TDT" },                      /* HDCDRV_SERVICE_TYPE_TDT */
    { "service_PROF" },                     /* HDCDRV_SERVICE_TYPE_PROF */
    { "service_IDE_file_trans" },           /* HDCDRV_SERVICE_TYPE_IDE_FILE_TRANS */
    { "service_dump" },                     /* HDCDRV_SERVICE_TYPE_DUMP */
    { "service_user3" },                    /* HDCDRV_SERVICE_TYPE_USER3 */
    { "service_DVPP" },                     /* HDCDRV_SERVICE_TYPE_DVPP */
    { "service_queue" },                    /* HDCDRV_SERVICE_TYPE_QUEUE */
    { "service_upgrade" },                  /* HDCDRV_SERVICE_TYPE_UPGRADE */
};

static void hdcdrv_fill_service_type_str(void)
{
    int i, ret;

    for (i = HDCDRV_SERVICE_TYPE_USER_START; i < HDCDRV_SUPPORT_MAX_SERVICE; i++) {
        ret = snprintf_s(hdcdrv_service_type_str[i].str, HDCDRV_STR_NAME_LEN, HDCDRV_STR_NAME_LEN - 1,
            "service_user_%d", i - HDCDRV_SERVICE_TYPE_USER_START);
        if (ret < 0) {
            hdcdrv_err("snprintf_s error, service %d can not print error string in log.\n",
                i - HDCDRV_SERVICE_TYPE_USER_START);
        }
    }
}

static struct {
    const char str[HDCDRV_STR_NAME_LEN];
} hdcdrv_close_state_str[HDCDRV_CLOSE_TYPE_MAX + 1] = {
    { "state_none" },                       /* HDCDRV_CLOSE_TYPE_NONE */
    { "closed_by_user" },                   /* HDCDRV_CLOSE_TYPE_USER */
    { "closed_by_user_form_kernel" },       /* HDCDRV_CLOSE_TYPE_KERNEL */
    { "closed_by_release" },                /* HDCDRV_CLOSE_TYPE_RELEASE */
    { "closed_by_set_owner_timeout" },      /* HDCDRV_CLOSE_TYPE_NOT_SET_OWNER */
    { "remote_close_pre" },                 /* HDCDRV_CLOSE_TYPE_REMOTE_CLOSED_PRE */
    { "remote_close_post" },                /* HDCDRV_CLOSE_TYPE_REMOTE_CLOSED_POST */
    { "state_max" }                         /* HDCDRV_CLOSE_TYPE_MAX */
};

void hdcdrv_get_mempool_size(u32 *small_packet_num, u32 *huge_packet_num)
{
#ifdef HDC_ENV_DEVICE
    *small_packet_num = HDCDRV_SMALL_PACKET_NUM;
    *huge_packet_num = HDCDRV_HUGE_PACKET_NUM;
#else
    u32 mempool_level = hdcdrv_get_hdc_mempool_level();
    if (mempool_level >= HDC_MEM_POOL_LEVEL_INVALID) {
        *small_packet_num = HDCDRV_SMALL_PACKET_NUM;
        *huge_packet_num = HDCDRV_HUGE_PACKET_NUM;
    } else {
       *small_packet_num = HDCDRV_SMALL_PACKET_NUM >> mempool_level;
       *huge_packet_num = HDCDRV_HUGE_PACKET_NUM >> mempool_level;
    }
#endif
}

/* only [vfio] and [mdev+sriov's vm] use workqueue, others use tasklet */
void hdcdrv_rx_msg_schedule_task(struct hdcdrv_msg_chan *msg_chan)
{
    if ((g_rx_notify_sched_mode == HDCDRV_RX_SCHED_WORK) || (devdrv_is_mdev_vm_boot_mode(msg_chan->dev_id) == true)) {
        queue_work(msg_chan->rx_workqueue, &msg_chan->rx_notify_work);
    } else {
        tasklet_schedule(&msg_chan->rx_notify_task);
    }
}

#if defined(CFG_FEATURE_VFIO) || defined(CFG_FEATURE_VFIO_DEVICE)
STATIC int hdcdrv_bandwidth_limit_check(struct hdcdrv_session *session, u32 dir, u32 data_len, u32 node_cnt)
{
    struct vmng_bandwidth_check_info check_info;

    check_info.dir = dir;
    check_info.vfid = session->local_fid;
    check_info.dev_id = (u32)session->dev_id;
    check_info.data_len = data_len;
    check_info.node_cnt = node_cnt;

#ifdef CFG_FEATURE_VFIO
    check_info.handle_mode = VMNG_BW_BANDWIDTH_CHECK_SLEEPABLE;
    return vmng_bandwidth_limit_check(&check_info);
#else
    check_info.handle_mode = VMNG_BW_BANDWIDTH_CHECK_NON_SLEEP;
    return vmng_bandwidth_limit_check(&check_info);
#endif

    return HDCDRV_ERR;
}
#endif

STATIC u32 hdcdrv_gen_unique_value(void)
{
    u32 value;

    value = (u32)atomic_inc_return(&hdc_ctrl->unique_val);
#ifdef HDC_ENV_DEVICE
    value = HDCDRV_SESSION_UNIQUE_VALUE_DEVICE_FLAG | (value & HDCDRV_SESSION_UNIQUE_VALUE_MASK);
#else
    value = HDCDRV_SESSION_UNIQUE_VALUE_HOST_FLAG | (value & HDCDRV_SESSION_UNIQUE_VALUE_MASK);
#endif
    return value;
}

STATIC int hdcdrv_session_pre_alloc(u32 dev_id, u32 fid, int service_type)
{
#ifdef CFG_FEATURE_VFIO
    if (fid != HDCDRV_DEFAULT_PM_FID) {
        return vhdch_session_pre_alloc(dev_id, fid, service_type);
    } else {
        return HDCDRV_OK;
    }
#else
    return HDCDRV_OK;
#endif
}

STATIC void hdcdrv_session_post_free(u32 dev_id, u32 fid, int service_type)
{
#ifdef CFG_FEATURE_VFIO
    if (fid != HDCDRV_DEFAULT_PM_FID) {
        vhdch_session_free(dev_id, fid, service_type);
    }
#endif
}

int hdcdrv_check_session_owner(const struct hdcdrv_session *session, u64 check_pid)
{
    if (session->owner_pid != check_pid) {
        return HDCDRV_NO_PERMISSION;
    }

    return HDCDRV_OK;
}

STATIC const char *hdcdrv_sevice_str(int service_type)
{
    if ((service_type < HDCDRV_SERVICE_TYPE_DMP) || (service_type >= HDCDRV_SUPPORT_MAX_SERVICE)) {
        return "service_invaild";
    } else if ((service_type < HDCDRV_SERVICE_TYPE_USER_START) && (service_type >= HDCDRV_SERVICE_TYPE_APPLY_MAX)) {
        return "service_reserved";
    } else {
        return hdcdrv_service_type_str[service_type].str;
    }
}

STATIC const char *hdcdrv_close_str(int close_state)
{
    if ((close_state < HDCDRV_CLOSE_TYPE_NONE) || (close_state > HDCDRV_CLOSE_TYPE_MAX)) {
        return hdcdrv_close_state_str[HDCDRV_CLOSE_TYPE_MAX].str;
    } else {
        return hdcdrv_close_state_str[close_state].str;
    }
}

STATIC void hdcdrv_print_status(int service_type)
{
    STATIC u32 print_limit_count[HDCDRV_SUPPORT_MAX_SERVICE] = {0};
    u32 count[HDCDRV_SESSION_STATUS_MAX] = {0};
    struct hdcdrv_session *sessions = NULL;
    u32 *server_count = NULL;
    int status;
    int index;
    int i;

    server_count = (u32 *)hdcdrv_kvmalloc(sizeof(u32) * HDCDRV_SESSION_STATUS_MAX * HDCDRV_SUPPORT_MAX_SERVICE);
    if (server_count == NULL) {
        hdcdrv_err("Calling kzalloc failed.\n");
        return;
    }

    mutex_lock(&hdc_ctrl->mutex);
    index = hdc_ctrl->cur_alloc_session;
    for (i = 0; i < HDCDRV_REAL_MAX_SESSION; i++) {
        sessions = &hdc_ctrl->sessions[i];
        status = hdcdrv_get_session_status(sessions);
        if (status == HDCDRV_SESSION_STATUS_IDLE) {
            count[HDCDRV_SESSION_STATUS_IDLE]++;
        } else if (status == HDCDRV_SESSION_STATUS_CONN) {
            count[HDCDRV_SESSION_STATUS_CONN]++;
            server_count[HDCDRV_SESSION_STATUS_CONN * HDCDRV_SUPPORT_MAX_SERVICE + sessions->service_type]++;
        } else if (status == HDCDRV_SESSION_STATUS_REMOTE_CLOSED) {
            count[HDCDRV_SESSION_STATUS_REMOTE_CLOSED]++;
            server_count[HDCDRV_SESSION_STATUS_REMOTE_CLOSED * HDCDRV_SUPPORT_MAX_SERVICE + sessions->service_type]++;
        } else {
            count[HDCDRV_SESSION_STATUS_CLOSING]++;
            server_count[HDCDRV_SESSION_STATUS_CLOSING * HDCDRV_SUPPORT_MAX_SERVICE + sessions->service_type]++;
        }
    }
    mutex_unlock(&hdc_ctrl->mutex);

    for (i = 0; i < HDCDRV_SERVICE_TYPE_APPLY_MAX; i++) {
        if ((print_limit_count[service_type] % HDCDRV_DFX_PRINT_LIMIT_CNT) == 0) {
            hdcdrv_err("Service:%s, conn cnt:%u; remote closing cnt:%u; closing cnt:%u\n", hdcdrv_sevice_str(i),
                server_count[HDCDRV_SESSION_STATUS_CONN * HDCDRV_SUPPORT_MAX_SERVICE + i],
                server_count[HDCDRV_SESSION_STATUS_REMOTE_CLOSED * HDCDRV_SUPPORT_MAX_SERVICE + i],
                server_count[HDCDRV_SESSION_STATUS_CLOSING * HDCDRV_SUPPORT_MAX_SERVICE + i]);
        }
    }
    hdcdrv_kvfree(server_count);
    print_limit_count[service_type]++;

    hdcdrv_err_limit("Get session info. (index=%d; state_idle=%u; connected=%u; remote_close=%u; others=%u)\n",
        index, count[HDCDRV_SESSION_STATUS_IDLE], count[HDCDRV_SESSION_STATUS_CONN],
        count[HDCDRV_SESSION_STATUS_REMOTE_CLOSED], count[HDCDRV_SESSION_STATUS_CLOSING]);
}

#ifdef CFG_FEATURE_MIRROR
int hdcdrv_alloc_mem_mirror(int pool_type, int dev_id, int len, struct hdcdrv_buf_desc *desc,
    struct list_head *wait_head)
{
    int ret;
    struct mirror_alloc_para para;
    int block_capacity = HDCDRV_HUGE_PACKET_SEGMENT - HDCDRV_MEM_BLOCK_HEAD_SIZE;

    para.dev_id = dev_id;
    para.len = len;
    para.wait_head = wait_head;

    if (get_pool_type(len) == HDCDRV_USE_POOL) {
        ret = alloc_mem(pool_type, dev_id, len, &desc->buf, &desc->addr, wait_head);
    } else {
        if (unlikely((len > block_capacity) || (len <= 0))) {
            hdcdrv_err_limit("data len invalid. (len=%d, capacity=%d)\n", len, block_capacity);
            return HDCDRV_ERR;
        }
        ret = alloc_mem_page(pool_type, para, &desc->buf, &desc->addr, &desc->mem_id);
    }

    return ret;
}

void hdcdrv_free_mem_mirror(void* buf, int mem_id, int len)
{
    if (get_pool_type(len) == HDCDRV_USE_POOL) {
        free_mem(buf);
    } else {
        free_mem_page(buf, mem_id);
    }
}
#endif

STATIC int hdcdrv_tx_alloc_mem(struct hdcdrv_session *session, const struct hdcdrv_cmd_send *cmd,
    struct hdcdrv_buf_desc *desc)
{
    int ret = HDCDRV_OK;

#ifdef CFG_FEATURE_VFIO
    struct hdccom_alloc_mem_para para = {0};

    if (session->owner == HDCDRV_SESSION_OWNER_VM) {
        if ((cmd->pool_buf == NULL) || (cmd->pool_addr == 0)) {
            hdcdrv_err("Input pararmeter is error.\n");
            return HDCDRV_PARA_ERR;
        }

        desc->buf = cmd->pool_buf;
        ret = hdcdrv_dma_map_guest_page((u32)session->dev_id, session->local_fid,
            cmd->pool_addr, (unsigned long)cmd->len, desc);
        return ret;
    }

    if (session->owner == HDCDRV_SESSION_OWNER_CT) {
        para.pool_type = HDCDRV_MEM_POOL_TYPE_TX;
        para.dev_id = session->dev_id;
        para.fid = session->local_fid;
        para.len = cmd->len;
        para.wait_head = NULL;
        ret = vhdch_alloc_mem_container(&para, desc);
        return ret;
    }
#endif

#ifdef CFG_FEATURE_MIRROR
    ret = hdcdrv_alloc_mem_mirror(HDCDRV_MEM_POOL_TYPE_TX, session->dev_id, cmd->len, desc, NULL);
#else
    ret = alloc_mem(HDCDRV_MEM_POOL_TYPE_TX, session->dev_id, cmd->len, &desc->buf, &desc->addr, NULL);
#endif
    return ret;
}

STATIC int hdcdrv_rx_alloc_mem(const struct hdcdrv_session *session, struct hdccom_alloc_mem_para *para,
    struct hdcdrv_buf_desc *desc)
{
    int ret;

#ifdef CFG_FEATURE_VFIO
    if (session->owner == HDCDRV_SESSION_OWNER_VM) {
        para->pool_type = HDCDRV_MEM_POOL_TYPE_RX;
        para->dev_id = session->dev_id;
        para->fid = session->local_fid;
        ret = vhdch_alloc_mem_vm(para, desc);
        return ret;
    }

    if (session->owner == HDCDRV_SESSION_OWNER_CT) {
        para->pool_type = HDCDRV_MEM_POOL_TYPE_RX;
        para->dev_id = session->dev_id;
        para->fid = session->local_fid;
        ret = vhdch_alloc_mem_container(para, desc);
        return ret;
    }
#endif

#ifdef CFG_FEATURE_MIRROR
    ret = hdcdrv_alloc_mem_mirror(HDCDRV_MEM_POOL_TYPE_RX, session->dev_id, para->len, desc, para->wait_head);
#else
    ret = alloc_mem(HDCDRV_MEM_POOL_TYPE_RX, session->dev_id, para->len,
        &desc->buf, &desc->addr, para->wait_head);
#endif
    return ret;
}

void hdcdrv_free_mem(struct hdcdrv_session *session, void *buf, int flag,
    struct hdcdrv_buf_desc *desc)
{
#ifdef CFG_FEATURE_VFIO
    struct hdcdrv_session_work_node *entry = NULL;

    if ((desc != NULL) && (desc->fid > HDCDRV_DEFAULT_PM_FID) && (session->owner == HDCDRV_SESSION_OWNER_VM)) {
        if (flag == HDCDRV_MEMPOOL_FREE_IN_PM) {
            entry = (struct hdcdrv_session_work_node *)kzalloc(sizeof(struct hdcdrv_session_work_node),
                GFP_ATOMIC | __GFP_ACCOUNT);
            if (entry == NULL) {
                hdcdrv_err_spinlock("Calling malloc failed.\n");
                return;
            }

            entry->buf = buf;
            entry->dma_sgt = desc->dma_sgt;
            entry->dev_id = desc->dev_id;
            entry->fid = desc->fid;

            spin_lock_bh(&session->session_work.lock);
            list_add(&entry->list, &session->session_work.root_list);
            spin_unlock_bh(&session->session_work.lock);

            schedule_work(&session->session_work.swork);
        } else {
            /* memory pool buffer will free in VM : vhdca_top_half_free */
            hdcdrv_dma_unmap_guest_page(desc->dev_id, desc->fid, desc->dma_sgt);
        }
        return;
    }

    if (session->owner == HDCDRV_SESSION_OWNER_CT) {
        vhdch_free_mem_container((u32)session->dev_id, session->local_fid, session->chan_id, buf);
        return;
    }
#endif
#ifdef CFG_FEATURE_MIRROR
    hdcdrv_free_mem_mirror(buf, desc->mem_id, desc->len);
#else
    free_mem(buf);
#endif
}

STATIC void hdcdrv_desc_clear(struct hdcdrv_buf_desc *desc)
{
    if (desc == NULL) {
        return;
    }

    desc->dma_sgt = NULL;
    desc->dev_id = 0;
    desc->fid = 0;
    desc->buf = NULL;
    desc->addr = 0;
    desc->len = 0;
}

STATIC int hdcdrv_copy_from_user(struct hdcdrv_session *session, void *dest,
    const struct hdcdrv_cmd_send *cmd, int mode)
{
    int ret = HDCDRV_OK;

#ifdef CFG_FEATURE_VFIO
    if (session->owner == HDCDRV_SESSION_OWNER_VM) {
        return ret;
    }
#endif

    if (mode == HDCDRV_MODE_USER) {
        if (copy_from_user(dest, (void __user *)cmd->src_buf, (unsigned long)cmd->len) != 0) {
            return HDCDRV_COPY_FROM_USER_FAIL;
        }
    } else {
        ret = memcpy_s(dest, (size_t)cmd->len, cmd->src_buf, (size_t)cmd->len);
    }

    return ret;
}

STATIC int hdcdrv_copy_to_user(const struct hdcdrv_session *session, struct hdcdrv_cmd_recv *cmd,
    int len, int mode, int* mem_id_list)
{
    int ret = HDCDRV_OK;
    int count = 0;
    unsigned int offset = 0;

#ifdef CFG_FEATURE_VFIO
    if (session->owner == HDCDRV_SESSION_OWNER_VM) {
        cmd->pool_buf = cmd->buf_list[0];
        cmd->out_len = len;
        ret = HDCDRV_CMD_CONTINUE;
        return ret;
    }
#endif

    if (mode == HDCDRV_MODE_USER) {
        for (count = 0; count < cmd->buf_count; count++) {
            if (copy_to_user((void __user *)cmd->dst_buf + offset, cmd->buf_list[count],
                (unsigned long)cmd->buf_len[count]) != 0) {
                hdcdrv_err("Calling copy_to_user failed. (dev_id=%d; session=%d)\n",  session->dev_id, cmd->session);
                ret = HDCDRV_COPY_TO_USER_FAIL;
            }
            offset += cmd->buf_len[count];
        }
    } else {
        // kernel_recv now use normal_recv, so the buf_index is fixed 0
        if (memcpy_s(cmd->dst_buf, (size_t)cmd->len, cmd->buf_list[0], (size_t)(u32)len) != EOK) {
            hdcdrv_err("Calling memcpy_s failed.\n");
            ret = HDCDRV_ERR;
        }
    }

#ifdef CFG_FEATURE_VFIO
    if (session->owner == HDCDRV_SESSION_OWNER_CT) {
        for (count = 0; count < cmd->buf_count; count++) {
            vhdch_free_mem_container((u32)session->dev_id, session->local_fid, session->chan_id, cmd->buf_list[count]);
            cmd->buf_list[count] = NULL;
        }
        cmd->out_len = len;
        return ret;
    }
#endif
    for (count = 0; count < cmd->buf_count; count++) {
#ifdef CFG_FEATURE_MIRROR
        hdcdrv_free_mem_mirror(cmd->buf_list[count], mem_id_list[count], cmd->buf_len[count]);
#else
        free_mem(cmd->buf_list[count]);
        (void)mem_id_list;
#endif
        cmd->buf_list[count] = NULL;
    }
    cmd->pool_buf = NULL;
    cmd->out_len = len;
    return ret;
}

int hdcdrv_get_service_level(int service_type)
{
    return hdc_ctrl->service_attr[service_type].level;
}

int hdcdrv_get_service_conn_feature(int service_type)
{
    return hdc_ctrl->service_attr[service_type].conn_feature;
}

int hdcdrv_service_level_init(int service_type)
{
    if ((service_type == HDCDRV_SERVICE_TYPE_RDMA) || (service_type == HDCDRV_SERVICE_TYPE_FRAMEWORK) ||
        (service_type == HDCDRV_SERVICE_TYPE_TSD) || (service_type == HDCDRV_SERVICE_TYPE_TDT)) {
        return HDCDRV_SERVICE_HIGH_LEVEL;
    }

    return HDCDRV_SERVICE_LOW_LEVEL;
}

int hdcdrv_service_conn_feature_init(int service_type)
{
    if (service_type == HDCDRV_SERVICE_TYPE_DMP) {
        return HDCDRV_SERVICE_SHORT_CONN;
    }

    return HDCDRV_SERVICE_LONG_CONN;
}

STATIC int hdcdrv_tasklet_status_check(struct hdcdrv_msg_chan_tasklet_status *tasklet_status)
{
    if (tasklet_status->schedule_in_last != tasklet_status->schedule_in) {
        tasklet_status->schedule_in_last = tasklet_status->schedule_in;
        tasklet_status->no_schedule_cnt = 0;
        return HDCDRV_OK;
    }

    tasklet_status->no_schedule_cnt++;

    if (tasklet_status->no_schedule_cnt <= HDCDRV_TASKLET_STATUS_CHECK_TIME) {
        return HDCDRV_OK;
    }

    tasklet_status->no_schedule_cnt = 0;

    return HDCDRV_ERR;
}

STATIC int hdcdrv_get_wait_flag(int timeout)
{
    int wait_flag;

    if (timeout < 0) {
        wait_flag = HDCDRV_WAIT_ALWAYS;
    } else if (timeout == 0) {
        wait_flag = HDCDRV_NOWAIT;
    } else {
        wait_flag = HDCDRV_WAIT_TIMEOUT;
    }

    return wait_flag;
}

struct hdcdrv_service *hdcdrv_search_service(u32 dev_id, u32 fid, int service_type, u64 host_pid)
{
    struct hdcdrv_serv_list_node *node = NULL;
    struct list_head *pos = NULL;
    struct list_head *next = NULL;
    struct hdcdrv_service *service = NULL;

#ifdef CFG_FEATURE_VFIO
    if ((fid != HDCDRV_DEFAULT_PM_FID) && (fid < VMNG_VDEV_MAX_PER_PDEV)) {
        return vhdch_search_service(dev_id, fid, service_type, host_pid);
    }
#endif

    service = &hdc_ctrl->devices[dev_id].service[service_type];

    if (hdc_ctrl->service_attr[service_type].service_scope == HDCDRV_SERVICE_SCOPE_GLOBAL) {
        return service;
    }

#ifdef CFG_FEATURE_VFIO_DEVICE
    if ((service_type == HDCDRV_SERVICE_TYPE_TDT) && (fid == HDCDRV_DEFAULT_PM_FID)) {
        return service;
    }
#endif

    if (list_empty_careful(&service->serv_list) == 0) {
        list_for_each_safe(pos, next, &service->serv_list) {
            node = list_entry(pos, struct hdcdrv_serv_list_node, list);
            if (node->service.listen_pid == host_pid) {
                return &node->service;
            }
        }
    }

    return service;
}

STATIC struct hdcdrv_service *hdcdrv_alloc_service(u32 dev_id, u32 fid, int service_type, u64 host_pid)
{
    struct hdcdrv_serv_list_node *node = NULL;
    struct list_head *pos = NULL;
    struct list_head *next = NULL;
    struct hdcdrv_service *service = NULL;
    struct hdcdrv_dev *dev = &hdc_ctrl->devices[dev_id];

#ifdef CFG_FEATURE_VFIO
    if ((fid > HDCDRV_DEFAULT_PM_FID) && (fid < VMNG_VDEV_MAX_PER_PDEV)) {
        return vhdch_alloc_service(dev_id, fid, service_type, host_pid);
    }
#endif

    service = &dev->service[service_type];

    if (hdc_ctrl->service_attr[service_type].service_scope == HDCDRV_SERVICE_SCOPE_GLOBAL) {
        return service;
    }

#ifdef CFG_FEATURE_VFIO_DEVICE
    if ((service_type == HDCDRV_SERVICE_TYPE_TDT) && (fid == HDCDRV_DEFAULT_PM_FID)) {
        return service;
    }
#endif

    mutex_lock(&dev->mutex);

    /* Checking whether the process server with the Same hostpid exists */
    if (list_empty_careful(&service->serv_list) == 0) {
        list_for_each_safe(pos, next, &service->serv_list) {
            node = list_entry(pos, struct hdcdrv_serv_list_node, list);
            if (node->service.listen_pid == host_pid) {
                mutex_unlock(&dev->mutex);
                return &node->service;
            }
        }
    }

    /* Searching for an idle server */
    if (list_empty_careful(&service->serv_list) == 0) {
        list_for_each_safe(pos, next, &service->serv_list) {
            node = list_entry(pos, struct hdcdrv_serv_list_node, list);
            if (node->service.listen_pid == HDCDRV_INVALID) {
                node->service.listen_pid = host_pid;
                mutex_unlock(&dev->mutex);
                return &node->service;
            }
        }
    }
    mutex_unlock(&dev->mutex);

    return NULL;
}

STATIC void hdcdrv_free_service(struct hdcdrv_service *service)
{
    mutex_lock(&service->mutex);
    service->listen_pid = HDCDRV_INVALID;
    mutex_unlock(&service->mutex);
}

STATIC void hdcdrv_delay_work_set(struct hdcdrv_session *session, struct delayed_work *work,
    unsigned int bit_offset, int timeout)
{
    int delay;

    if (hdc_ctrl->debug_state.valid == HDCDRV_VALID) {
        delay = HDCDRV_DEBUG_MODE_TIMEOUT;
    } else {
        delay = timeout;
    }

    spin_lock_bh(&session->lock);
    if ((session->delay_work_flag & BIT(bit_offset)) != 0) {
        spin_unlock_bh(&session->lock);
        hdcdrv_warn("Session work is setted. (flag_offset=%d)\n", bit_offset);
        return;
    }
    session->delay_work_flag |= BIT(bit_offset);
    spin_unlock_bh(&session->lock);

    (void)schedule_delayed_work(work, (unsigned long)delay);
}

STATIC void hdcdrv_delay_work_cancel(struct hdcdrv_session *session, struct delayed_work *work,
    unsigned int bit_offset)
{
    spin_lock_bh(&session->lock);
    if (!(session->delay_work_flag & BIT(bit_offset))) {
        spin_unlock_bh(&session->lock);
        return;
    }

    session->delay_work_flag &= ~BIT(bit_offset);
    spin_unlock_bh(&session->lock);

    (void)cancel_delayed_work_sync(work);
}

STATIC void hdcdrv_delay_work_flag_clear(struct hdcdrv_session *session, unsigned int bit_offset)
{
    spin_lock_bh(&session->lock);
    session->delay_work_flag &= ~BIT(bit_offset);
    spin_unlock_bh(&session->lock);
}

STATIC struct hdcdrv_service *hdcdrv_link_ctrl_msg_get_service(u32 devid, int service_type,
    enum HDC_LINK_CTRL_MSG_STATUS_TYPE status_type)
{
    struct hdcdrv_dev *dev = NULL;

    if ((status_type < HDCDRV_LINK_CTRL_MSG_SEND_SUCC) || (status_type >= HDCDRV_LINK_CTRL_MSG_STATUS_MAX)) {
        return NULL;
    }

    if (hdcdrv_dev_para_check((int)devid, service_type) != HDCDRV_OK) {
        return NULL;
    }

    dev = &hdc_ctrl->devices[devid];
    return &dev->service[service_type];
}

STATIC inline void hdcdrv_conncet_ctrl_msg_stats_add(struct hdcdrv_service *service,
    enum HDC_LINK_CTRL_MSG_STATUS_TYPE status_type, int last_err)
{
    service->service_stat.connect_msg_stat.count[status_type]++;
    if (last_err != HDCDRV_OK) {
        service->service_stat.connect_msg_stat.last_err[status_type] = last_err;
    }
}

STATIC inline void hdcdrv_reply_ctrl_msg_stats_add(struct hdcdrv_service *service,
    enum HDC_LINK_CTRL_MSG_STATUS_TYPE status_type, int last_err)
{
    service->service_stat.reply_msg_stat.count[status_type]++;
    if (last_err != HDCDRV_OK) {
        service->service_stat.reply_msg_stat.last_err[status_type] = last_err;
    }
}

STATIC inline void hdcdrv_close_ctrl_msg_stats_add(struct hdcdrv_service *service,
    enum HDC_LINK_CTRL_MSG_STATUS_TYPE status_type, int last_err)
{
    service->service_stat.close_msg_stat.count[status_type]++;
    if (last_err != HDCDRV_OK) {
        service->service_stat.close_msg_stat.last_err[status_type] = last_err;
    }
}

STATIC void hdcdrv_link_ctrl_msg_stats_add(u32 devid, int service_type, u32 msg_type,
    enum HDC_LINK_CTRL_MSG_STATUS_TYPE status_type, int last_err)
{
    struct hdcdrv_service *service = NULL;

    service = hdcdrv_link_ctrl_msg_get_service(devid, service_type, status_type);
    if (service == NULL) {
        return;
    }

    mutex_lock(&service->mutex);
    switch (msg_type) {
        case HDCDRV_CTRL_MSG_TYPE_CONNECT:
            hdcdrv_conncet_ctrl_msg_stats_add(service, status_type, last_err);
            break;
        case HDCDRV_CTRL_MSG_TYPE_CONNECT_REPLY:
            hdcdrv_reply_ctrl_msg_stats_add(service, status_type, last_err);
            break;
        case HDCDRV_CTRL_MSG_TYPE_CLOSE:
            hdcdrv_close_ctrl_msg_stats_add(service, status_type, last_err);
            break;
        default:
            break;
    }
    mutex_unlock(&service->mutex);
}

long hdcdrv_session_alive_check(int session_fd, int dev_id, u32 unique_val)
{
    struct hdcdrv_session *session = NULL;

    if ((session_fd >= HDCDRV_REAL_MAX_SESSION) || (session_fd < 0)) {
        hdcdrv_err("session_fd is illegal. (session_fd=%d)\n", session_fd);
        return HDCDRV_PARA_ERR;
    }

    session = &hdc_ctrl->sessions[session_fd];

    if (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_IDLE) {
        hdcdrv_link_ctrl_msg_stats_add((u32)dev_id, session->service_type, HDCDRV_CTRL_MSG_TYPE_CONNECT_REPLY,
            HDCDRV_LINK_CTRL_MSG_RECV_FAIL, session->local_close_state);
        hdcdrv_err("Session has already closed. (session=%d; dev_id=%d; service_type=\"%s\";"
            "local_close_state=\"%s\"; remote_close_state=\"%s\"; local_session_fd=%d; remote_session_fd=%d)\n",
            session_fd, session->dev_id, hdcdrv_sevice_str(session->service_type),
            hdcdrv_close_str(session->local_close_state), hdcdrv_close_str(session->remote_close_state),
            session->local_session_fd, session->remote_session_fd);
        return HDCDRV_SESSION_HAS_CLOSED;
    }

    if ((session->dev_id != dev_id) || (session->unique_val != unique_val)) {
        hdcdrv_link_ctrl_msg_stats_add((u32)dev_id, session->service_type, HDCDRV_CTRL_MSG_TYPE_CONNECT_REPLY,
            HDCDRV_LINK_CTRL_MSG_RECV_FAIL, HDCDRV_SESSION_ID_MISS_MATCH);
        hdcdrv_warn("Session has already closed. (session_fd=%d; dev_id=%d; session_devid=%d;"
            "unique_val=%u; session_unique_val=%u)\n", session_fd, dev_id, session->dev_id,
            unique_val, session->unique_val);
        return HDCDRV_SESSION_ID_MISS_MATCH;
    }

    return HDCDRV_OK;
}

STATIC long hdcdrv_session_state_to_remote_close(int session_fd, u32 unique_val)
{
    struct hdcdrv_session *session = NULL;
    int session_state;

    mutex_lock(&hdc_ctrl->mutex);
    session = &hdc_ctrl->sessions[session_fd];

    session_state = hdcdrv_get_session_status(session);
    if ((session_state == HDCDRV_SESSION_STATUS_IDLE) ||
        (session_state == HDCDRV_SESSION_STATUS_REMOTE_CLOSED) ||
        (session_state == HDCDRV_SESSION_STATUS_CLOSING)) {
        mutex_unlock(&hdc_ctrl->mutex);
        return HDCDRV_SESSION_HAS_CLOSED;
    }

    if (session->unique_val != unique_val) {
        mutex_unlock(&hdc_ctrl->mutex);
        hdcdrv_warn("session_fd %d, unique_val miss match\n", session->local_session_fd);
        return HDCDRV_SESSION_ID_MISS_MATCH;
    }

    hdcdrv_set_session_status(session, HDCDRV_SESSION_STATUS_REMOTE_CLOSED);
    mutex_unlock(&hdc_ctrl->mutex);

    return HDCDRV_OK;
}

STATIC long hdcdrv_session_state_to_closing(int session_fd, u64 pid, int *last_status,
    int close_state)
{
    struct hdcdrv_session *session = NULL;
    int session_state;

    mutex_lock(&hdc_ctrl->mutex);

    session = &hdc_ctrl->sessions[session_fd];

    session_state = hdcdrv_get_session_status(session);
    if ((session_state == HDCDRV_SESSION_STATUS_IDLE) ||
        (session_state == HDCDRV_SESSION_STATUS_CLOSING)) {
        mutex_unlock(&hdc_ctrl->mutex);
        if (session->service_type != HDCDRV_SERVICE_TYPE_DMP) {
            hdcdrv_warn_limit("Status IDLE, session has already closed. (session=%d; dev_id=%d; service_type=\"%s\";"
                " local_close_state=\"%s\"; remote_close_state=\"%s\"; local_session_fd=%d; remote_session_fd=%d; "
                "session_state=%d)\n", session_fd, session->dev_id, hdcdrv_sevice_str(session->service_type),
                hdcdrv_close_str(session->local_close_state), hdcdrv_close_str(session->remote_close_state),
                session->local_session_fd, session->remote_session_fd, session_state);
        }
        return HDCDRV_SESSION_HAS_CLOSED;
    }

    if (session->owner_pid != pid) {
        mutex_unlock(&hdc_ctrl->mutex);
        hdcdrv_warn_limit("owner_pid not match. (session=%d; pid=%llu; close_pid=%llu)\n", session_fd,
            session->owner_pid, pid);
        return HDCDRV_PARA_ERR;
    }

    if ((close_state == HDCDRV_CLOSE_TYPE_USER) && (hdcdrv_get_container_id() != session->container_id)) {
        mutex_unlock(&hdc_ctrl->mutex);
        hdcdrv_err("Container id verify failed. (session=%d)\n", session_fd);
        return HDCDRV_PARA_ERR;
    }

    *last_status = session_state;
    hdcdrv_set_session_status(session, HDCDRV_SESSION_STATUS_CLOSING);

    mutex_unlock(&hdc_ctrl->mutex);

    return HDCDRV_OK;
}

long hdcdrv_session_para_check(int session_fd, int device_id)
{
    struct hdcdrv_session *session = NULL;

    if ((session_fd >= HDCDRV_REAL_MAX_SESSION) || (session_fd < 0)) {
        hdcdrv_err_limit("session_fd is illegal. (session_fd=%d)\n", session_fd);
        return HDCDRV_PARA_ERR;
    }

    session = &hdc_ctrl->sessions[session_fd];

    if (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_IDLE) {
        hdcdrv_warn_limit("session_fd is invalid. (session_fd=%d; dev_id=%d; service_type=\"%s\";"
            "local_close_state=\"%s\"; remote_close_state=\"%s\"; local_session_fd=%d; remote_session_fd=%d)\n",
            session_fd, session->dev_id, hdcdrv_sevice_str(session->service_type),
            hdcdrv_close_str(session->local_close_state), hdcdrv_close_str(session->remote_close_state),
            session->local_session_fd, session->remote_session_fd);
        return HDCDRV_SESSION_HAS_CLOSED;
    }

    if ((session->owner == HDCDRV_SESSION_OWNER_VM) && (session->dev_id != device_id)) {
        hdcdrv_err("dev_id verify failed. (session=%d)\n", session_fd);
        return HDCDRV_PARA_ERR;
    }

    if (hdcdrv_get_container_id() != session->container_id) {
        hdcdrv_err("Container ID verify failed. (session=%d)\n", session_fd);
        return HDCDRV_PARA_ERR;
    }

    return HDCDRV_OK;
}

long hdcdrv_session_valid_check(int session_fd, int device_id, u64 check_pid)
{
    struct hdcdrv_session *session = NULL;
    long ret;

    ret = hdcdrv_session_para_check(session_fd, device_id);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    session = &hdc_ctrl->sessions[session_fd];
    ret = hdcdrv_check_session_owner(session, check_pid);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    return HDCDRV_OK;
}

long hdcdrv_session_inner_check(int session_fd, u32 checker)
{
    struct hdcdrv_session *session = NULL;

    if ((session_fd >= HDCDRV_REAL_MAX_SESSION) || (session_fd < 0)) {
        hdcdrv_err("Input pararmeter is error. (session_fd=%d)\n", session_fd);
        return HDCDRV_PARA_ERR;
    }

    session = &hdc_ctrl->sessions[session_fd];
    if (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_IDLE) {
        hdcdrv_warn("Status IDLE, session has already closed. (session=%d; dev_id=%d; service_type=\"%s\";"
            "local_close_state=\"%s\"; remote_close_state=\"%s\"; local_session_fd=%d; remote_session_fd=%d)\n",
            session_fd, session->dev_id, hdcdrv_sevice_str(session->service_type),
            hdcdrv_close_str(session->local_close_state), hdcdrv_close_str(session->remote_close_state),
            session->local_session_fd, session->remote_session_fd);
        return HDCDRV_PARA_ERR;
    }

    if (session->inner_checker != checker) {
        hdcdrv_warn("Session inner check not match, session may closed before. (session_fd=%d)\n", session_fd);
        return HDCDRV_PARA_ERR;
    }

    return HDCDRV_OK;
}

STATIC struct hdcdrv_session_fast_rx *hdcdrv_get_session_fast_rx(int session)
{
    return &hdc_ctrl->sessions[session].fast_rx;
}

STATIC void hdcdrv_bind_sessoin_ctx(struct hdcdrv_ctx *ctx, struct hdcdrv_session *session)
{
    if (ctx == HDCDRV_KERNEL_WITHOUT_CTX) {
        return;
    }

    ctx->session_fd = session->local_session_fd;
    ctx->pid = session->owner_pid;
    ctx->service = NULL;
    ctx->session = session;
    ctx->dev_id = session->dev_id;
    session->ctx = ctx;

    return;
}

STATIC void hdcdrv_unbind_session_ctx(struct hdcdrv_session *session)
{
    if (session->ctx != NULL) {
        session->ctx->session = NULL;
        session->ctx = NULL;
    }
}

STATIC void hdcdrv_bind_server_ctx(struct hdcdrv_ctx *ctx, struct hdcdrv_service *service,
    int dev_id, int service_type)
{
    if (ctx == HDCDRV_KERNEL_WITHOUT_CTX) {
        return;
    }

    ctx->dev_id = dev_id;
    ctx->pid = service->listen_pid;
    ctx->service_type = service_type;
    ctx->session = NULL;
    ctx->service = service;

    service->ctx = ctx;

    return;
}

STATIC void hdcdrv_unbind_server_ctx(struct hdcdrv_service *server)
{
    if (server->ctx != NULL) {
        server->ctx->service = NULL;
        server->ctx = NULL;
    }
}

STATIC u64 hdcdrv_set_send_timeout(const struct hdcdrv_session *session, unsigned int timeout)
{
    if (timeout == 0) {
        return session->timeout_jiffies.send_timeout;
    } else {
        return msecs_to_jiffies(timeout);
    }
}

STATIC long hdcdrv_set_fast_send_timeout(const struct hdcdrv_session *session, unsigned int timeout)
{
    if (timeout == 0) {
        return (long)session->timeout_jiffies.fast_send_timeout;
    } else {
        return (long)msecs_to_jiffies(timeout);
    }
}

STATIC void hdcdrv_inner_checker_set(struct hdcdrv_session *session)
{
    session->inner_checker = session->unique_val;
}

STATIC void hdcdrv_inner_checker_clear(struct hdcdrv_session *session)
{
    session->inner_checker = 0;
}

STATIC void hdcdrv_fill_sq_desc(struct hdcdrv_sq_desc *sq_desc,
    const struct hdcdrv_session *session, struct hdcdrv_buf_desc *buf_d, int type)
{
    sq_desc->local_session = session->local_session_fd;
    sq_desc->remote_session = session->remote_session_fd;
    sq_desc->src_data_addr = (u64)buf_d->addr;
    sq_desc->data_len = buf_d->len;
    sq_desc->inner_checker = session->unique_val;

    if (type == HDCDRV_MSG_CHAN_TYPE_FAST) {
        sq_desc->dst_data_addr = buf_d->dst_data_addr;
        sq_desc->dst_ctrl_addr = buf_d->dst_ctrl_addr;
        sq_desc->ctrl_len = buf_d->ctrl_len;
        sq_desc->src_fid = buf_d->fid;
        sq_desc->src_pid = buf_d->pid;
        sq_desc->src_data_addr = buf_d->src_data_addr_va;
        sq_desc->src_ctrl_addr = buf_d->src_ctrl_addr_va;
    }

    wmb();
    sq_desc->valid = HDCDRV_VALID;
}

STATIC long hdcdrv_msg_chan_send_wait(struct hdcdrv_msg_chan *msg_chan, struct hdcdrv_session *session,
    int wait_flag, u64 timeout)
{
    long ret;
    struct hdcdrv_dev *dev = NULL;

    dev = &hdc_ctrl->devices[msg_chan->dev_id];

    if (wait_flag == HDCDRV_NOWAIT) {
        return HDCDRV_NO_BLOCK;
    } else if (wait_flag == HDCDRV_WAIT_ALWAYS) {
        /*lint -e666*/
        ret = (long)wait_event_interruptible_exclusive(msg_chan->send_wait,
            (!(hdcdrv_w_sq_full_check(msg_chan->chan))) ||
            (dev->valid != HDCDRV_VALID) ||
            (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_IDLE));
        /*lint +e666*/
    } else {
        /*lint -e666*/
        ret = wait_event_interruptible_timeout(msg_chan->send_wait,
            (!(hdcdrv_w_sq_full_check(msg_chan->chan))) ||
            (dev->valid != HDCDRV_VALID) ||
            (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_IDLE),
            (long)timeout);
        /*lint +e666*/
        if (ret == 0) {
            return HDCDRV_TX_TIMEOUT;
        } else if (ret > 0) {
            ret = 0;
        }
    }

    if (ret != 0) {
        hdcdrv_warn("Get wait_ret. (device=%u; session=%d; service_type=\"%s\"; wait_ret=%ld)\n", dev->dev_id,
            session->local_session_fd, hdcdrv_sevice_str(session->service_type), ret);
        return ret;
    }

    if (dev->valid != HDCDRV_VALID) {
        /* Clear the packets in the waiting queue */
        wake_up_interruptible(&msg_chan->send_wait);
        hdcdrv_err("Device reset. (session=%d; service_type=\"%s\"; device=%u)\n",
            session->local_session_fd, hdcdrv_sevice_str(session->service_type), dev->dev_id);

        return HDCDRV_DEVICE_RESET;
    }

    if (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_IDLE) {
        hdcdrv_warn("Session status is idle. (device=%u; chan_id=%d; session=%d; service_type=\"%s\"; "
            "local_close_state=\"%s\"; remote_close_state=\"%s\"; local_session_fd=%d; remote_session_fd=%d)\n",
            dev->dev_id, msg_chan->chan_id, session->local_session_fd, hdcdrv_sevice_str(session->service_type),
            hdcdrv_close_str(session->local_close_state), hdcdrv_close_str(session->remote_close_state),
            session->local_session_fd, session->remote_session_fd);
        return HDCDRV_SESSION_HAS_CLOSED;
    }

    return HDCDRV_OK;
}

STATIC long hdcdrv_msg_chan_send(struct hdcdrv_msg_chan *msg_chan, struct hdcdrv_buf_desc *buf_d,
    int wait_flag, u64 timeout)
{
    struct hdcdrv_session *session = NULL;
    struct hdcdrv_service *service = NULL;
    struct hdcdrv_sq_desc *sq_desc = NULL;
    struct hdcdrv_dev *dev = NULL;
    u32 tail = 0;
    long ret;

    session = &hdc_ctrl->sessions[buf_d->local_session];
    service = session->service;
    dev = &hdc_ctrl->devices[msg_chan->dev_id];

    mutex_lock(&msg_chan->mutex);
#ifndef HDC_ENV_DEVICE
    ret = (long)hdcdrv_get_msgchan_refcnt(msg_chan->dev_id);
    if (ret != 0) {
        hdcdrv_err("Can not send message while hot reset flag is set. (ret=%ld, dev_id=%d)\n", ret, msg_chan->dev_id);
        mutex_unlock(&msg_chan->mutex);
        return ret;
    }
#endif
    msg_chan->dbg_stat.hdcdrv_msg_chan_send1++;

    /* Wait until there is space to send */
    while (hdcdrv_w_sq_full_check(msg_chan->chan)) {
        msg_chan->stat.tx_full++;
        session->stat.tx_full++;
        service->data_stat.tx_full++;

        mutex_unlock(&msg_chan->mutex);
        ret = hdcdrv_msg_chan_send_wait(msg_chan, session, wait_flag, timeout);
        if (ret != HDCDRV_OK) {
            return ret;
        }
        mutex_lock(&msg_chan->mutex);
    }

    sq_desc = hdcdrv_get_w_sq_desc(msg_chan->chan, &tail);
    if (sq_desc == NULL) {
        hdcdrv_err("Calling hdcdrv_get_w_sq_desc failed.\n");
        mutex_unlock(&msg_chan->mutex);
        return HDCDRV_SQ_DESC_NULL;
    }

    /* stored locally for release after subsequent sending */
    msg_chan->tx[tail] = *buf_d;

    hdcdrv_fill_sq_desc(sq_desc, session, buf_d, msg_chan->type);

    msg_chan->stat.tx++;
    msg_chan->stat.tx_bytes += (u32)buf_d->len;
    session->stat.tx++;
    session->stat.tx_bytes += (u32)buf_d->len;
    service->data_stat.tx++;
    service->data_stat.tx_bytes += (u32)buf_d->len;

    /* data and ctrl use diff dma channel */
    hdcdrv_copy_sq_desc_to_remote(msg_chan, sq_desc, DEVDRV_DMA_DATA_TRAFFIC);

    mutex_unlock(&msg_chan->mutex);

    return HDCDRV_OK;
}

#ifdef CFG_FEATURE_HDC_REG_MEM
STATIC void hdcdrv_tx_add_mem_to_wait_fifo(struct hdcdrv_session *session, struct hdcdrv_buf_desc *tx_desc)
{
    struct hdcdrv_wait_mem_fin_msg in_msg = {
        .dataAddr = tx_desc->src_data_addr_va,
        .dataLen = (u32)tx_desc->len,
        .ctrlAddr = tx_desc->src_ctrl_addr_va,
        .ctrlLen = (u32)tx_desc->ctrl_len
    };

    (void)hdcdrv_update_session_release_mem_event(session, &in_msg);
}
#endif

STATIC void hdcdrv_tx_finish_handle(struct hdcdrv_msg_chan *msg_chan, const struct hdcdrv_cq_desc *cq_desc)
{
    struct hdcdrv_buf_desc *tx_desc = NULL;
    u16 tx_head = (u16)cq_desc->sq_head;
    struct hdcdrv_session *session = NULL;
    struct hdcdrv_service *service = NULL;

    if ((cq_desc->session >= HDCDRV_REAL_MAX_SESSION) || (cq_desc->session < 0)) {
        msg_chan->dbg_stat.hdcdrv_tx_finish_notify_task6++;
        hdcdrv_err("Input pararmeter is error. (device=%u; session=%d; sq_head=%d)\n",
            msg_chan->dev_id, cq_desc->session, tx_head);
        goto OUT;
    }

    session = &hdc_ctrl->sessions[cq_desc->session];
    tx_desc = &msg_chan->tx[tx_head];

    if (msg_chan->type == HDCDRV_MSG_CHAN_TYPE_NORMAL) {
        if (unlikely(tx_desc->buf == NULL)) {
            msg_chan->dbg_stat.hdcdrv_tx_finish_notify_task5++;
            session->dbg_stat.hdcdrv_tx_finish_notify_task5++;
            hdcdrv_err("tx_desc_buf is invalid. (device=%u; session=%d; tx_head=%d)\n",
                msg_chan->dev_id, cq_desc->session, tx_head);
            goto OUT;
        }

        hdcdrv_free_mem(session, tx_desc->buf, HDCDRV_MEMPOOL_FREE_IN_PM, tx_desc);
    }
#ifdef CFG_FEATURE_HDC_REG_MEM
    if (msg_chan->type == HDCDRV_MSG_CHAN_TYPE_FAST) {
        hdcdrv_tx_add_mem_to_wait_fifo(session, tx_desc);
    }
#endif

    service = session->service;
    if (cq_desc->status != 0) {
        msg_chan->stat.tx_fail++;
        session->stat.tx_fail++;
        service->data_stat.tx_fail++;

        hdcdrv_err("TX failed. (device=%u; session=%d; service_type=\"%s\"; tx_head=%d; status=%u)\n",
            msg_chan->dev_id, cq_desc->session, hdcdrv_sevice_str(session->service_type), tx_head, cq_desc->status);
    } else {
        msg_chan->stat.tx_finish++;
        session->stat.tx_finish++;
        service->data_stat.tx_finish++;
    }
    session->dbg_stat.hdcdrv_tx_finish_notify_task7++;

OUT:
    hdcdrv_desc_clear(tx_desc);
}

void hdcdrv_tx_finish_notify_task(unsigned long data)
{
    struct hdcdrv_msg_chan *msg_chan = (struct hdcdrv_msg_chan *)((uintptr_t)data);
    u16 tx_head;
    int cnt = 0;

    struct hdcdrv_cq_desc *cq_desc = NULL;

    msg_chan->tx_finish_task_status.schedule_in++;

    msg_chan->dbg_stat.hdcdrv_tx_finish_notify_task1++;
    do {
        cq_desc = hdcdrv_get_r_cq_desc(msg_chan->chan);
        if ((cq_desc == NULL) || (cq_desc->valid != HDCDRV_VALID)) {
            break;
        }

        rmb();

        msg_chan->dbg_stat.hdcdrv_tx_finish_notify_task2++;

        /* Reach the threshold and schedule out */
        if (cnt >= HDCDRV_TX_BUDGET) {
            msg_chan->dbg_stat.hdcdrv_tx_finish_notify_task3++;
            tasklet_schedule(&msg_chan->tx_finish_task);
            break;
        }

        tx_head = (u16)cq_desc->sq_head;

        /* the branch should not enter */
        if (tx_head >= HDCDRV_DESC_QUEUE_DEPTH) {
            msg_chan->dbg_stat.hdcdrv_tx_finish_notify_task4++;
            hdcdrv_err("tx_head is invalid. (device=%d; sq_head=%d; session_id=%d)\n",
                msg_chan->dev_id, tx_head, cq_desc->session);
            break;
        }

        hdcdrv_tx_finish_handle(msg_chan, cq_desc);

        cq_desc->session = HDCDRV_SESSION_FD_INVALID;
        cq_desc->valid = HDCDRV_INVALID;
        hdcdrv_move_r_cq_desc(msg_chan->chan);

        /* update the sq head pointer to continue send packet */
        tx_head = (tx_head + 1) % HDCDRV_DESC_QUEUE_DEPTH;
        hdcdrv_set_w_sq_desc_head(msg_chan->chan, tx_head);
        msg_chan->sq_head = tx_head;
        cnt++;
        wmb();
        wake_up_interruptible(&msg_chan->send_wait);
        msg_chan->dbg_stat.hdcdrv_tx_finish_notify_task7++;

#ifndef HDC_ENV_DEVICE
        (void)hdcdrv_put_msgchan_refcnt(msg_chan->dev_id);
#endif
    } while (1);
}

void hdcdrv_tx_finish_notify(void *chan)
{
    struct hdcdrv_dev *dev = NULL;
    struct hdcdrv_msg_chan *msg_chan = hdcdrv_get_msg_chan_priv(chan);

    if (msg_chan == NULL) {
        hdcdrv_err("Input pararmeter is error.");
        return;
    }

    dev = &hdc_ctrl->devices[msg_chan->dev_id];

    if (dev->valid != HDCDRV_VALID) {
        hdcdrv_err("tx_finish_notify device reset. (dev_id=%u)\n", dev->dev_id);
        return;
    }

    msg_chan->dbg_stat.hdcdrv_tx_finish_notify1++;

    tasklet_schedule(&msg_chan->tx_finish_task);
}

STATIC void hdcdrv_tx_finish_task_check(struct hdcdrv_msg_chan *msg_chan)
{
    struct hdcdrv_msg_chan_tasklet_status *tasklet_status = NULL;
    struct hdcdrv_cq_desc *cq_desc = NULL;

    tasklet_status = &msg_chan->tx_finish_task_status;

    msg_chan->dbg_stat.hdcdrv_tx_finish_task_check1++;

    if (hdcdrv_tasklet_status_check(tasklet_status) == HDCDRV_OK) {
        return;
    }

    cq_desc = hdcdrv_get_r_cq_desc(msg_chan->chan);
    if (cq_desc == NULL) {
        return;
    }

    /* no valid cq */
    if (cq_desc->valid != HDCDRV_VALID) {
        return;
    }

    msg_chan->dbg_stat.hdcdrv_tx_finish_task_check2++;

    /* has finish cq but not schedule */
    tasklet_schedule(&msg_chan->tx_finish_task);
}

void hdcdrv_response_cq(struct hdcdrv_msg_chan *msg_chan, u32 sq_head, u32 status, int local_session,
                        int remote_session)
{
    struct hdcdrv_cq_desc *cq_desc = NULL;

    /* response cq */
    cq_desc = hdcdrv_get_w_cq_desc(msg_chan->chan);
    if (cq_desc == NULL) {
        hdcdrv_err("Calling hdcdrv_get_w_cq_desc error. (dev_id=%u)\n", msg_chan->dev_id);
        return;
    }

    cq_desc->sq_head = sq_head;
    cq_desc->status = status;
    cq_desc->session = remote_session;
    wmb();
    cq_desc->valid = HDCDRV_VALID;

    /* data and ctrl use diff dma channel */
    hdcdrv_copy_cq_desc_to_remote(msg_chan, cq_desc, DEVDRV_DMA_DATA_TRAFFIC);
}

void hdcdrv_update_msg_chan_head(struct hdcdrv_msg_chan *msg_chan, u32 sq_head,
    u32 status, int local_session, int remote_session)
{
    msg_chan->rx[sq_head].buf = NULL;
    msg_chan->rx[sq_head].addr = 0;
    msg_chan->rx[sq_head].len = 0;

    hdcdrv_response_cq(msg_chan, sq_head, status, local_session, remote_session);

    msg_chan->rx_head = (int)sq_head;
}

STATIC bool hdcdrv_session_rx_list_is_full(const int session_fd, const int chan_type)
{
    struct hdcdrv_session *session = &hdc_ctrl->sessions[session_fd];

    if (chan_type == HDCDRV_MSG_CHAN_TYPE_NORMAL) {
        return ((session->normal_rx.tail + 1) % HDCDRV_SESSION_RX_LIST_MAX_PKT == session->normal_rx.head);
    }

    return ((session->fast_rx.tail + 1) % HDCDRV_BUF_MAX_CNT == session->fast_rx.head);
}

STATIC int hdcdrv_rebuild_notify_pid(u64 owner_pid, u64 create_pid)
{
    u64 local_pid;

    local_pid = (owner_pid == HDCDRV_INVALID_PID) ? create_pid : owner_pid;

    return hdcdrv_rebuild_raw_pid(local_pid);
}
STATIC void hdcdrv_msg_chan_recv_update_status(struct hdcdrv_msg_chan *msg_chan,
    struct hdcdrv_service *service, struct hdcdrv_session *session, const struct hdcdrv_buf_desc *rx_desc)
{
    msg_chan->stat.rx++;
    msg_chan->stat.rx_bytes += (unsigned long long)rx_desc->len;
    session->stat.rx++;
    session->stat.rx_bytes += (unsigned long long)rx_desc->len;
    service->data_stat.rx++;
    service->data_stat.rx_bytes += (unsigned long long)rx_desc->len;
}

STATIC int hdcdrv_connect_notify(int service_type, int dev_id, int vfid, int peer_pid, int local_pid)
{
    int ret = HDCDRV_OK;

    if (g_session_notify[service_type].connect_notify == NULL) {
        return ret;
    }

    ret = g_session_notify[service_type].connect_notify(dev_id, vfid, peer_pid, local_pid);
    if (ret != HDCDRV_OK) {
        hdcdrv_err("Connect notify failed. (dev=%d; vfid=%d; peer_pid=%d; local_pid=%d; service_type=\"%s\")\n",
            dev_id, vfid, peer_pid, local_pid, hdcdrv_sevice_str(service_type));
    }

    return ret;
}

STATIC int hdcdrv_close_notify(int service_type, int dev_id, int vfid, int peer_pid, int local_pid)
{
    int ret = HDCDRV_OK;

    if (g_session_notify[service_type].close_notify == NULL) {
        return ret;
    }

    ret = g_session_notify[service_type].close_notify(dev_id, vfid, peer_pid, local_pid);
    if (ret != HDCDRV_OK) {
        hdcdrv_err("Close notify failed. (dev=%d; vfid=%d; peer_pid=%d; local_pid=%d; service_type=\"%s\")\n",
            dev_id, vfid, peer_pid, local_pid, hdcdrv_sevice_str(service_type));
    }

    return ret;
}

STATIC int hdcdrv_data_in_notify(int service_type, int dev_id, int vfid, int local_pid,
    struct hdcdrv_data_info data_info)
{
    int ret = HDCDRV_RX_CONTINUE;

    if (g_session_notify[service_type].data_in_notify == NULL) {
        return ret;
    }

    ret = g_session_notify[service_type].data_in_notify(dev_id, vfid, local_pid, data_info);

    return ret;
}

STATIC int hdcdrv_msg_chan_recv_handle(struct hdcdrv_msg_chan *msg_chan, struct hdcdrv_buf_desc *rx_desc)
{
    struct hdcdrv_fast_rx *rx_buf = NULL;
    struct hdcdrv_session *session = NULL;
    struct hdcdrv_service *service = NULL;
    struct hdcdrv_data_info data_info = {0};
    int ret;

    session = &hdc_ctrl->sessions[rx_desc->local_session];
    service = session->service;

    spin_lock_bh(&session->lock);
    data_info.data_type = msg_chan->type;
    data_info.src_addr =
        ((msg_chan->type == HDCDRV_MSG_CHAN_TYPE_NORMAL) ? (u64)(uintptr_t)rx_desc->buf : rx_desc->dst_data_addr);
    data_info.len = (u32)rx_desc->len;
    data_info.session_fd = rx_desc->local_session;

    ret = hdcdrv_data_in_notify(session->service_type, session->dev_id, (int)session->remote_fid,
        hdcdrv_rebuild_notify_pid(session->owner_pid, session->create_pid), data_info);
    if (ret == HDCDRV_RX_FINISH) {
        hdcdrv_msg_chan_recv_update_status(msg_chan, service, session, rx_desc);
        spin_unlock_bh(&session->lock);
        if (msg_chan->type == HDCDRV_MSG_CHAN_TYPE_NORMAL) {
            hdcdrv_free_mem(session, rx_desc->buf, HDCDRV_MEMPOOL_FREE_IN_PM, rx_desc);
        }
        return HDCDRV_OK;
    }

    /* The session received too many packets, stop recv...
       Wait for the upper layer to take the packet first
       Note: There will be mutual influence between different
       sessions in the same queue */
    if (hdcdrv_session_rx_list_is_full(rx_desc->local_session, msg_chan->type)) {
        msg_chan->dbg_stat.hdcdrv_msg_chan_recv_task6++;
        msg_chan->stat.rx_full++;
        session->stat.rx_full++;
        session->dbg_stat.hdcdrv_msg_chan_recv_task6++;
        service->data_stat.rx_full++;
        msg_chan->rx_trigger_flag = HDCDRV_VALID;
        spin_unlock_bh(&session->lock);
        if (ret != HDCDRV_RX_CONTINUE) {
            hdcdrv_err("Get data_in_notify ret. (dev=%d; vfid=%d; local_pid=%d; service_type=\"%s\"; ret=%d)\n",
                session->dev_id, session->remote_fid, hdcdrv_rebuild_notify_pid(session->owner_pid,
                session->create_pid), hdcdrv_sevice_str(session->service_type), ret);
        }
        return HDCDRV_ERR;
    }

    /* update stats */
    hdcdrv_msg_chan_recv_update_status(msg_chan, service, session, rx_desc);

    /* Insert into the tail of the receive list */
    if (msg_chan->type == HDCDRV_MSG_CHAN_TYPE_NORMAL) {
        session->normal_rx.rx_list[session->normal_rx.tail] = *rx_desc;
        session->normal_rx.tail = (session->normal_rx.tail + 1) % HDCDRV_SESSION_RX_LIST_MAX_PKT;
    } else {
        rx_buf = &session->fast_rx.rx_list[session->fast_rx.tail];
        rx_buf->data_addr = rx_desc->dst_data_addr;
        rx_buf->ctrl_addr = rx_desc->dst_ctrl_addr;
        rx_buf->data_len = rx_desc->len;
        rx_buf->ctrl_len = rx_desc->ctrl_len;
        session->fast_rx.tail = (session->fast_rx.tail + 1) % HDCDRV_BUF_MAX_CNT;
    }
    spin_unlock_bh(&session->lock);

    if (ret != HDCDRV_RX_CONTINUE) {
        hdcdrv_err("Get data_in_notify ret. (dev=%d; vfid=%d; local_pid=%d; service_type=\"%s\"; ret=%d)\n",
            session->dev_id, session->remote_fid, hdcdrv_rebuild_notify_pid(session->owner_pid,
            session->create_pid), hdcdrv_sevice_str(session->service_type), ret);
    }

    session->dbg_stat.hdcdrv_msg_chan_recv_task7++;
    msg_chan->dbg_stat.hdcdrv_msg_chan_recv_task7++;
    wmb();

    /* Trigger the upper reception process */
    wake_up_interruptible(&session->wq_rx);

    hdcdrv_epoll_wake_up(session->epfd);

    return HDCDRV_OK;
}

void hdcdrv_msg_chan_recv_task(unsigned long data)
{
    struct hdcdrv_msg_chan *msg_chan = (struct hdcdrv_msg_chan *)((uintptr_t)data);
    struct hdcdrv_buf_desc *rx_desc = NULL;
    struct hdcdrv_session *session = NULL;
    void *buf = NULL;
    int rx_head, status, local_session, remote_session;
    long ret;

    msg_chan->rx_task_status.schedule_in++;
    msg_chan->dbg_stat.hdcdrv_msg_chan_recv_task1++;
    while (msg_chan->rx_head != msg_chan->dma_head) {
        rx_head = (msg_chan->rx_head + 1) % HDCDRV_DESC_QUEUE_DEPTH;
        rx_desc = &msg_chan->rx[rx_head];
        rmb();
        buf = rx_desc->buf;
        status = rx_desc->status;
        msg_chan->dbg_stat.hdcdrv_msg_chan_recv_task2++;

        local_session = rx_desc->local_session;
        remote_session = rx_desc->remote_session;

        if (rx_desc->skip_flag == HDCDRV_VALID) {
            msg_chan->dbg_stat.hdcdrv_msg_chan_recv_task3++;
            goto NEXT;
        }

        /* This branch should not enter */
        if (unlikely((msg_chan->type == HDCDRV_MSG_CHAN_TYPE_NORMAL) && (buf == NULL))) {
            msg_chan->dbg_stat.hdcdrv_msg_chan_recv_task4++;
            hdcdrv_err("Parameter buf is null. (dev=%u; session=%d; rx_head=%d)\n",
                msg_chan->dev_id, local_session, rx_head);
            msg_chan->rx_head = rx_head;
            continue;
        }

        ret = hdcdrv_session_inner_check(local_session, rx_desc->inner_checker);
        if ((ret != HDCDRV_OK) || (status != 0)) {
            msg_chan->stat.rx_fail++;
            msg_chan->dbg_stat.hdcdrv_msg_chan_recv_task5++;
            if (msg_chan->type == HDCDRV_MSG_CHAN_TYPE_NORMAL) {
                session = &hdc_ctrl->sessions[local_session];
                session->dbg_stat.hdcdrv_msg_chan_recv_task5++;
                hdcdrv_free_mem(session, buf, HDCDRV_MEMPOOL_FREE_IN_PM, rx_desc);
            }
            hdcdrv_warn("Calling session_inner_check not success. (device=%u; local_session=%d; "
                "remote_session=%d; chan_id=%d; rx_head=%d; status=%u; ret=%ld)\n",
                msg_chan->dev_id, local_session, remote_session, msg_chan->chan_id, rx_head, status, ret);
            goto NEXT;
        }

        if (hdcdrv_msg_chan_recv_handle(msg_chan, rx_desc) != HDCDRV_OK) {
            break;
        }

    NEXT:
        hdcdrv_desc_clear(rx_desc);
        hdcdrv_response_cq(msg_chan, (u32)rx_head, (u32)status, local_session, remote_session);
        msg_chan->rx_head = rx_head;
    }
}

STATIC void hdcdrv_rx_msg_callback(void *data, u32 trans_id, u32 status)
{
    struct hdcdrv_msg_chan *msg_chan = (struct hdcdrv_msg_chan *)data;
    struct hdcdrv_dev *dev = &hdc_ctrl->devices[msg_chan->dev_id];
    struct hdcdrv_session *session;

    if (msg_chan->dma_need_submit_flag == HDCDRV_VALID) {
        hdcdrv_rx_msg_schedule_task(msg_chan);
        hdcdrv_info("Device message channel schedule. (dev=%d; msg_chan=%d)\n", dev->dev_id, msg_chan->chan_id);
    }

    msg_chan->dbg_stat.hdcdrv_rx_msg_callback1++;
    /* This branch should not enter */
    if (unlikely(trans_id >= HDCDRV_DESC_QUEUE_DEPTH)) {
        msg_chan->dbg_stat.hdcdrv_rx_msg_callback2++;
        hdcdrv_err("trans_id is invalid. (device=%u; trans_id=%u)\n", dev->dev_id, trans_id);
        return;
    }

    session = &hdc_ctrl->sessions[msg_chan->rx[trans_id].local_session];
    session->dbg_stat.hdcdrv_rx_msg_callback1++;

    if (msg_chan->type == HDCDRV_MSG_CHAN_TYPE_FAST) {
        hdcdrv_node_status_idle_by_mem(msg_chan->rx[trans_id].src_data);
        hdcdrv_node_status_idle_by_mem(msg_chan->rx[trans_id].dst_data);
        hdcdrv_node_status_idle_by_mem(msg_chan->rx[trans_id].src_ctrl);
        hdcdrv_node_status_idle_by_mem(msg_chan->rx[trans_id].dst_ctrl);

        msg_chan->rx[trans_id].src_data = NULL;
        msg_chan->rx[trans_id].dst_data = NULL;
        msg_chan->rx[trans_id].src_ctrl = NULL;
        msg_chan->rx[trans_id].dst_ctrl = NULL;
    }

    if (dev->valid != HDCDRV_VALID) {
        hdcdrv_err("Rx msg callback device reset. (dev_id=%u)\n", dev->dev_id);
        return;
    }

    msg_chan->rx[trans_id].status = (int)status;

    wmb();
    msg_chan->dma_head = (int)trans_id;
    msg_chan->dbg_stat.hdcdrv_rx_msg_callback3++;
    session->dbg_stat.hdcdrv_rx_msg_callback3++;
    tasklet_schedule(&msg_chan->rx_task);
}

STATIC void hdcdrv_rx_msg_task_check(struct hdcdrv_msg_chan *msg_chan)
{
    struct hdcdrv_msg_chan_tasklet_status *tasklet_status = NULL;

    tasklet_status = &msg_chan->rx_task_status;

    msg_chan->dbg_stat.hdcdrv_rx_msg_task_check1++;

    if (hdcdrv_tasklet_status_check(tasklet_status) == HDCDRV_OK) {
        return;
    }

    /* no valid msg */
    if (msg_chan->rx_head == msg_chan->dma_head) {
        return;
    }

    msg_chan->dbg_stat.hdcdrv_rx_msg_task_check2++;

    /* has msg but not schedule */
    tasklet_schedule(&msg_chan->rx_task);
}

STATIC int hdcdrv_normal_dma_copy(struct hdcdrv_msg_chan *msg_chan, u32 head, const struct hdcdrv_sq_desc *sq_desc,
    struct devdrv_asyn_dma_para_info *para, enum devdrv_dma_data_type data_type)
{
    /* the receiving end cannot apply for memory, too many receive
       buffers, and waits for the upper layer to receive packets.
       Block live send.
       malloc fixed len memory to avoid memory fragmentation */
    struct hdccom_alloc_mem_para alloc_para;
    struct hdcdrv_buf_desc *rx_desc = &msg_chan->rx[head];
    struct hdcdrv_session *session = &hdc_ctrl->sessions[sq_desc->remote_session];
    int ret;

#if defined(CFG_FEATURE_VFIO) || defined(CFG_FEATURE_VFIO_DEVICE)
    ret = hdcdrv_bandwidth_limit_check(session, hdcdrv_get_dma_direction(), (u32)sq_desc->data_len, 1);
    if (ret != HDCDRV_OK) {
        return HDCDRV_DMA_QUE_FULL;
    }
#endif

    alloc_para.len = sq_desc->data_len;
    alloc_para.wait_head = &msg_chan->wait_mem_list;
    ret = hdcdrv_rx_alloc_mem(session, &alloc_para, rx_desc);
    if (ret != HDCDRV_OK) {
        session->dbg_stat.hdcdrv_normal_dma_copy1++;
        msg_chan->dbg_stat.hdcdrv_normal_dma_copy1++;
        return HDCDRV_DMA_MEM_ALLOC_FAIL;
    }

    /* record recv buf desc */
    rx_desc->len = sq_desc->data_len;
    rx_desc->local_session = sq_desc->remote_session;
    rx_desc->remote_session = sq_desc->local_session;

    /* copy the packet */
    ret = devdrv_dma_async_copy_plus(msg_chan->dev_id, data_type, msg_chan->chan_id,
        (dma_addr_t)sq_desc->src_data_addr, rx_desc->addr, (u32)sq_desc->data_len, hdcdrv_get_dma_direction(), para);
    if (ret == -ENOSPC) {
        hdcdrv_free_mem(session, rx_desc->buf, HDCDRV_MEMPOOL_FREE_IN_PM, rx_desc);
        ret = HDCDRV_DMA_QUE_FULL;
    } else  if (ret != 0) {
        ret = HDCDRV_DMA_COPY_FAIL;
    }
    return ret;
}

STATIC u64 hdcdrv_rebuild_hash(u32 rb_side, u64 hash_va, u32 fid)
{
    u64 hash_va_new = hash_va;
    u64 fid_t = (u32)fid;

    if (rb_side == HDCDRV_RBTREE_SIDE_LOCAL) {
        hash_va_new = (fid_t << HDCDRV_FRBTREE_FID_BEG) | (hash_va & HDCDRV_HASH_VA_PIDADDR_MASK);
    }

    return hash_va_new;
}

void* hdcdrv_get_sync_mem_buf(int dev_id)
{
    return hdc_ctrl->devices[dev_id].sync_mem_buf;
}

struct mutex *hdcdrv_get_sync_mem_lock(int dev_id)
{
    return &(hdc_ctrl->devices[dev_id].sync_mem_mutex);
}

static void hdcdrv_get_fnode_from_msg(struct hdcdrv_fast_node *f_node, u64 hash_va_wfid,
    const struct hdcdrv_ctrl_msg_sync_mem_info *msg, int devid)
{
    u32 i;

    f_node->fast_mem.devid = devid;
    f_node->fast_mem.phy_addr_num = msg->phy_addr_num;
    f_node->fast_mem.alloc_len = msg->alloc_len;
    f_node->fast_mem.mem_type = msg->mem_type;
    f_node->fast_mem.page_type = HDCDRV_PAGE_TYPE_NONE;
    f_node->fast_mem.hash_va = hash_va_wfid;
    f_node->pid = msg->pid;
    f_node->hash_va = hash_va_wfid;
#ifdef CFG_FEATURE_HDC_REG_MEM
    f_node->fast_mem.align_size = msg->align_size;
    f_node->fast_mem.register_inner_page_offset = msg->register_offset;
    f_node->fast_mem.user_va = msg->user_va;
#endif

    for (i = 0; i < (u32)msg->phy_addr_num; i++) {
        f_node->fast_mem.mem[i].addr = msg->mem[i].addr;
        f_node->fast_mem.mem[i].len = msg->mem[i].len;
    }
}

STATIC struct hdcdrv_fast_node *hdcdrv_remote_node_alloc(int num)
{
    struct hdcdrv_fast_node *f_node = NULL;
    struct hdcdrv_mem_f* mem = NULL;
    u64 len_mem;

    f_node = (struct hdcdrv_fast_node *)kzalloc(sizeof(struct hdcdrv_fast_node), GFP_KERNEL | __GFP_ACCOUNT);
    if (f_node == NULL) {
        hdcdrv_err("Calling kzalloc failed.\n");
        return NULL;
    }

    len_mem = (u64)sizeof(struct hdcdrv_mem_f) * (u32)num;
    mem = (struct hdcdrv_mem_f *)kzalloc(len_mem, GFP_KERNEL | __GFP_ACCOUNT);
    if (mem == NULL) {
        hdcdrv_fast_node_free(f_node);
        hdcdrv_err("Calling kzalloc failed. (size=%lld)\n", len_mem);
        return NULL;
    }

    f_node->fast_mem.mem = mem;
    return f_node;
}

STATIC void hdcdrv_fast_node_msg_info_fill(int devid,
    const struct hdcdrv_ctrl_msg_sync_mem_info *sync_msg, struct hdcdrv_fast_node_msg_info *msg)
{
    msg->dev_id = (u32)devid;
    msg->pid = (u64)sync_msg->pid;
    msg->fid = (u32)(sync_msg->hash_va >> HDCDRV_FRBTREE_FID_BEG) & HDCDRV_FRBTREE_FID_MASK;

    msg->len = sync_msg->alloc_len;
#ifdef CFG_FEATURE_HDC_REG_MEM
    msg->va_addr = sync_msg->user_va;
#endif
    msg->hash_val = sync_msg->hash_va;
}

int hdcdrv_add_mem_info(int devid, u32 fid, u32 rb_side, struct hdcdrv_ctrl_msg_sync_mem_info *msg)
{
    struct hdcdrv_fast_node *f_node = NULL;
    int ret;
    u32 fid_tmp;
    bool fast_node_flg = false;
    struct hdcdrv_fast_node_msg_info node_msg;
    struct hdcdrv_dev_fmem *dev_fmem = hdcdrv_get_dev_fmem_sep(devid);
    struct rb_root* rbtree = hdcdrv_get_rbtree(dev_fmem, rb_side);
    u64 hash_va_wfid = hdcdrv_rebuild_hash(rb_side, msg->hash_va, fid);

    f_node = hdcdrv_fast_node_search_timeout(&dev_fmem->rb_lock, rbtree,
                                             hash_va_wfid, HDCDRV_NODE_WAIT_TIME_MAX);
    if ((f_node == NULL) && (msg->flag == HDCDRV_ADD_REGISTER_FLAG)) {
        hdcdrv_fast_node_msg_info_fill(devid, msg, &node_msg);
        f_node = hdcdrv_fast_node_search_timeout_from_arry(rb_side, HDCDRV_NODE_WAIT_TIME_MAX, &node_msg);
        fast_node_flg = true;
    }
    if (f_node != NULL) {
        hdcdrv_warn("This fast node has exit, free it firstly.\n");
        if (fast_node_flg) {
            hdcdrv_fast_node_erase_from_arry((u64)f_node->pid, fid, devid, rb_side, f_node);
        } else {
            hdcdrv_fast_node_erase(&dev_fmem->rb_lock, rbtree, f_node);
        }
        hdcdrv_node_status_idle(f_node);
        hdcdrv_kvfree(f_node->fast_mem.mem);
        hdcdrv_fast_node_free(f_node);
    }

    f_node = hdcdrv_remote_node_alloc(msg->phy_addr_num);
    if (f_node == NULL) {
        hdcdrv_err("Calling kzalloc failed. (dev=%d)\n", devid);
        return HDCDRV_MEM_ALLOC_FAIL;
    }

    hdcdrv_get_fnode_from_msg(f_node, hash_va_wfid, msg, devid);

    /* if add rb is not remote, so is vm mode, map iova and send to remote. */
    if (rb_side == HDCDRV_RBTREE_SIDE_LOCAL) {
        ret = hdcdrv_iova_fmem_map((u32)devid, fid, &(f_node->fast_mem));
        if (ret != HDCDRV_OK) {
            hdcdrv_err("Calling hdcdrv_iova_fmem_map failed. (dev=%d; fid=%u; ret=%d)\n", devid, fid, ret);
            goto IOVA_FAIL;
        }

        ret = hdcdrv_send_mem_info(&(f_node->fast_mem), devid, HDCDRV_ADD_FLAG);
        if (ret != HDCDRV_OK) {
            hdcdrv_err("Calling hdcdrv_send_mem_info failed. (dev=%d; fid=%u; ret=%d)\n", devid, fid, ret);
            goto ADD_REMOTE_FAIL;
        }
    }

    if (msg->flag == HDCDRV_ADD_REGISTER_FLAG) {
        fid_tmp = (u32)((msg->hash_va >> HDCDRV_FRBTREE_FID_BEG) & HDCDRV_FRBTREE_FID_MASK);
        ret = hdcdrv_fast_node_insert_to_arry(devid, (u64)msg->pid, fid_tmp, rb_side, f_node);
        if (ret != HDCDRV_OK) {
            hdcdrv_err_limit("fast insert fail. (devid=%d; msg_type=%d; hash_va=0x%llx; len=%d; pid=0x%llx)\n",
                devid, msg->type, msg->hash_va, msg->alloc_len, msg->pid);
            goto INS_FAIL;
        }
    } else {
        ret = hdcdrv_fast_node_insert(&dev_fmem->rb_lock, rbtree, f_node);
        if (ret != HDCDRV_OK) {
            hdcdrv_err_limit("Insert failed, node exist. (devid=%d; msg_type=%d; flag=%d; mem_num=%d; pid=0x%llx)\n",
                devid, msg->type, msg->flag, msg->phy_addr_num, msg->pid);
            goto INS_FAIL;
        }
    }
    return HDCDRV_OK;

INS_FAIL:
    if (rb_side == HDCDRV_RBTREE_SIDE_LOCAL) {
        ret = hdcdrv_send_mem_info(&(f_node->fast_mem), devid, HDCDRV_DEL_FLAG);
    }
ADD_REMOTE_FAIL:
    if (rb_side == HDCDRV_RBTREE_SIDE_LOCAL) {
        hdcdrv_iova_fmem_unmap((u32)devid, fid, &(f_node->fast_mem), (u32)f_node->fast_mem.phy_addr_num);
    }
IOVA_FAIL:
    hdcdrv_kvfree(f_node->fast_mem.mem);
    hdcdrv_fast_node_free(f_node);
    return ret;
}

STATIC int hdcdrv_del_mem_info(int devid, u32 fid, u32 rb_side, const struct hdcdrv_ctrl_msg_sync_mem_info *msg)
{
    struct hdcdrv_fast_node *f_node = NULL;
    struct hdcdrv_dev_fmem *dev_fmem = hdcdrv_get_dev_fmem_sep(devid);
    struct rb_root* rbtree = hdcdrv_get_rbtree(dev_fmem, rb_side);
    u64 hash_va_wfid = hdcdrv_rebuild_hash(rb_side, msg->hash_va, fid);
    int ret;
    bool fast_node_flg = false;
    struct hdcdrv_fast_node_msg_info node_msg;

    f_node = hdcdrv_fast_node_search_timeout(&dev_fmem->rb_lock, rbtree,
                                             hash_va_wfid, HDCDRV_NODE_WAIT_TIME_MAX);
    if ((f_node == NULL) && (msg->flag == HDCDRV_DEL_REGISTER_FLAG)) {
        hdcdrv_fast_node_msg_info_fill(devid, msg, &node_msg);
        f_node = hdcdrv_fast_node_search_timeout_from_arry(rb_side, HDCDRV_NODE_WAIT_TIME_MAX, &node_msg);
        fast_node_flg = true;
    }
    if (f_node == NULL) {
        hdcdrv_err("Calling hdcdrv_fast_node_search_timeout failed. (dev=%d rb_side=%d hash=0x%llx)\n",
            devid, rb_side, hash_va_wfid);
        return HDCDRV_F_NODE_SEARCH_FAIL;
    }

    /* if add rb is not remote, so is vm mode, map iova and send to remote. */
    if (rb_side == HDCDRV_RBTREE_SIDE_LOCAL) {
        ret = hdcdrv_send_mem_info(&(f_node->fast_mem), devid, HDCDRV_DEL_FLAG);
        if (ret != HDCDRV_OK) {
            hdcdrv_err("Calling hdcdrv_send_mem_info failed. (dev=%d; fid=%u; ret=%d)\n", devid, fid, ret);
        }
        hdcdrv_iova_fmem_unmap((u32)devid, fid, &(f_node->fast_mem), (u32)f_node->fast_mem.phy_addr_num);
    }

    if (fast_node_flg) {
        hdcdrv_fast_node_erase_from_arry((u64)f_node->pid, fid, devid, rb_side, f_node);
    } else {
        hdcdrv_fast_node_erase(&dev_fmem->rb_lock, rbtree, f_node);
    }

    hdcdrv_node_status_idle(f_node);
    hdcdrv_kvfree(f_node->fast_mem.mem);
    hdcdrv_fast_node_free(f_node);

    return HDCDRV_OK;
}

int hdcdrv_set_mem_info(int devid, u32 fid, u32 rb_side, struct hdcdrv_ctrl_msg_sync_mem_info *msg)
{
    if ((devid >= hdcdrv_get_max_support_dev())
#ifdef CFG_FEATURE_VFIO
        || (fid >= VMNG_VDEV_MAX_PER_PDEV)
#endif
        ) {
        hdcdrv_err("Input pararmeter is error. (dev_id=%d; fid=%u)\n", devid, fid);
        return HDCDRV_PARA_ERR;
    }

    if ((u32)msg->phy_addr_num > HDCDRV_MEM_MAX_PHY_NUM) {
        hdcdrv_err("Input pararmeter is error. (dev_id=%d; phy_addr_num=%d)\n", devid, msg->phy_addr_num);
        return HDCDRV_PARA_ERR;
    }

    if ((msg->flag == HDCDRV_ADD_FLAG) || (msg->flag == HDCDRV_ADD_REGISTER_FLAG)) {
        return hdcdrv_add_mem_info(devid, fid, rb_side, msg);
    } else {
        return hdcdrv_del_mem_info(devid, fid, rb_side, msg);
    }
}

STATIC void hdcdrv_fast_mem_sep_uninit(spinlock_t *lock, const struct rb_root *root)
{
    struct rb_node *node = NULL;
    struct hdcdrv_fast_node *f_node = NULL;
    int ret = 0;
    int devid;
    u32 fid;

    /* only uninit free, suspend status not free */
    if (hdcdrv_get_running_status() != HDCDRV_RUNNING_NORMAL) {
        return;
    }

    spin_lock_bh(lock);

    node = rb_first(root);
    while (node != NULL) {
        f_node = rb_entry(node, struct hdcdrv_fast_node, node);
        node = rb_next(node);

        devid = f_node->fast_mem.devid;
        fid = (u32)(f_node->fast_mem.hash_va >> HDCDRV_FRBTREE_FID_BEG);
        spin_unlock_bh(lock);
        ret = hdcdrv_send_mem_info(&(f_node->fast_mem), devid, HDCDRV_DEL_FLAG);
        if (ret != HDCDRV_OK) {
            hdcdrv_err("Calling hdcdrv_send_mem_info failed. (dev=%d; fid=%u; ret=%d)\n", devid, fid, ret);
        }
        hdcdrv_iova_fmem_unmap((u32)devid, fid, &(f_node->fast_mem), (u32)f_node->fast_mem.phy_addr_num);
        spin_lock_bh(lock);
    }

    spin_unlock_bh(lock);
}

int hdcdrv_mem_adapter(const struct hdcdrv_fast_mem *src, const struct hdcdrv_fast_mem *dst,
    struct devdrv_dma_node *node, int *node_idx, int len)
{
    int idx;
    u32 total_len;
    enum devdrv_dma_direction dir;
#ifdef CFG_FEATURE_HDC_REG_MEM
    u32 i = src->page_start_idx;
    u32 j = dst->page_start_idx;
    u32 src_offset = src->page_start_idx == 0 ?\
                    (src->send_inner_page_offset - src->register_inner_page_offset) : src->send_inner_page_offset;
    u32 dst_offset = dst->page_start_idx == 0 ?\
                    (dst->send_inner_page_offset - dst->register_inner_page_offset) : dst->send_inner_page_offset;
    dma_addr_t src_addr = (dma_addr_t)src->mem[i].addr + src_offset;
    dma_addr_t dst_addr = (dma_addr_t)dst->mem[j].addr + dst_offset;
    u32 src_len = src->mem[i].len - src_offset;
    u32 dst_len = dst->mem[j].len - dst_offset;
#else
    u32 i = 0;
    u32 j = 0;
    dma_addr_t src_addr = (dma_addr_t)src->mem[i].addr;
    dma_addr_t dst_addr = (dma_addr_t)dst->mem[j].addr;
    u32 src_len = src->mem[i].len;
    u32 dst_len = dst->mem[j].len;
#endif
    i++;
    j++;

    dir = hdcdrv_get_dma_direction();
    total_len = (u32)len;
    if ((total_len > dst->alloc_len) || (total_len > src->alloc_len)) {
        hdcdrv_err("Input pararmeter is error. (send_len=%d; src_len %d; dst_len=%d)\n",
            total_len, src->alloc_len, dst->alloc_len);
        return HDCDRV_PARA_ERR;
    }

    while (total_len > 0) {
        idx = *node_idx;
        if (idx >= HDCDRV_MAX_DMA_NODE) {
            hdcdrv_err("Need too many nodes, out of the range.\n");
            return HDCDRV_PARA_ERR;
        }

        if (src_len == dst_len) {
            if (total_len < src_len) {
                src_len = total_len;
            }

            node[idx].size = src_len;
            node[idx].src_addr = src_addr;
            node[idx].dst_addr = dst_addr;
            total_len -= src_len;

            if (total_len == 0) {
                goto FINISH;
            }

            src_addr = (dma_addr_t)src->mem[i].addr;
            src_len = src->mem[i].len;
            i++;

            dst_addr = (dma_addr_t)dst->mem[j].addr;
            dst_len = dst->mem[j].len;
            j++;
        } else if (src_len < dst_len) {
            if (total_len < src_len) {
                src_len = total_len;
            }

            node[idx].size = src_len;
            node[idx].src_addr = src_addr;
            node[idx].dst_addr = dst_addr;
            total_len -= src_len;

            if (total_len == 0) {
                goto FINISH;
            }

            dst_addr += src_len;
            dst_len -= src_len;

            src_addr = (dma_addr_t)src->mem[i].addr;
            src_len = src->mem[i].len;
            i++;
        } else {
            if (total_len < dst_len) {
                dst_len = total_len;
            }

            node[idx].size = dst_len;
            node[idx].src_addr = src_addr;
            node[idx].dst_addr = dst_addr;
            total_len -= dst_len;

            if (total_len == 0) {
                goto FINISH;
            }

            src_addr += dst_len;
            src_len -= dst_len;

            dst_addr = (dma_addr_t)dst->mem[j].addr;
            dst_len = dst->mem[j].len;
            j++;
        }

    FINISH:
        node[idx].direction = dir;
        *node_idx = (*node_idx) + 1;
    }

    return HDCDRV_OK;
}

int hdcdrv_get_dma_node_info(struct hdcdrv_fast_mem *src, struct hdcdrv_fast_mem *dst,
    struct hdcdrv_msg_chan *msg_chan, int *node_idx, int len)
{
    if ((src != NULL) && (dst != NULL)) {
        return hdcdrv_mem_adapter(src, dst, msg_chan->node, node_idx, len);
    }

    return HDCDRV_OK;
}

STATIC void hdcdrv_set_node_to_idle(struct hdcdrv_fast_mem_info *fmem_info)
{
    hdcdrv_node_status_idle_by_mem(fmem_info->src_data);
    hdcdrv_node_status_idle_by_mem(fmem_info->dst_data);
    hdcdrv_node_status_idle_by_mem(fmem_info->src_ctrl);
    hdcdrv_node_status_idle_by_mem(fmem_info->dst_ctrl);
}

STATIC int hdcdrv_fast_mem_valid_check(const struct hdcdrv_sq_desc *sq_desc, const struct hdcdrv_fast_mem *mem_src,
    const struct hdcdrv_fast_mem *mem_dst)
{
    if ((mem_src != NULL) && (mem_dst == NULL)) {
        hdcdrv_err("Fast dst buffer error. (local_session_id=%d; remote_session_id=%d; dst_buf=0x%llx)\n",
            sq_desc->remote_session, sq_desc->local_session, sq_desc->dst_ctrl_addr);
        return HDCDRV_PARA_ERR;
    }

    if ((mem_src == NULL) && (mem_dst != NULL)) {
        hdcdrv_err("Fast src buffer error. (local_session_id=%d; remote_session_id=%d; src_buf=0x%llx)\n",
            sq_desc->remote_session, sq_desc->local_session, sq_desc->src_ctrl_addr);
        return HDCDRV_PARA_ERR;
    }

    if ((mem_dst != NULL) && (mem_dst->dma_map == 0)) {
        hdcdrv_err("Fast dst buffer not map. (local_session_id=%d; remote_session_id=%d; dst_data_buf=0x%llx)\n",
            sq_desc->remote_session, sq_desc->local_session, sq_desc->dst_ctrl_addr);
        return HDCDRV_PARA_ERR;
    }

    return HDCDRV_OK;
}
STATIC int hdcdrv_fast_mem_check(struct hdcdrv_sq_desc *sq_desc, const struct hdcdrv_fast_mem *mem_src_data,
    const struct hdcdrv_fast_mem *mem_src_ctrl, struct hdcdrv_fast_mem *mem_dst_data,
    struct hdcdrv_fast_mem *mem_dst_ctrl)
{
    int ret;

    if ((mem_src_data == NULL) && (mem_dst_data == NULL) && (mem_src_ctrl == NULL) && (mem_dst_ctrl == NULL)) {
        hdcdrv_err("Input pararmeter is error. (local_session_id=%d; remote_session_id=%d; "
            "remote_data_addr=0x%llx; remote_ctrl_addr=0x%llx; local_data_addr=0x%llx; local_ctrl_addr=0x%llx)\n",
            sq_desc->remote_session, sq_desc->local_session, sq_desc->src_data_addr,
            sq_desc->src_ctrl_addr, sq_desc->dst_data_addr, sq_desc->dst_ctrl_addr);
        return HDCDRV_PARA_ERR;
    }

    ret = hdcdrv_fast_mem_valid_check(sq_desc, mem_src_data, mem_dst_data);
    if (ret != HDCDRV_OK) {
        hdcdrv_err("Calling hdcdrv_fast_mem_valid_check failed.\n");
        return HDCDRV_PARA_ERR;
    }

    ret = hdcdrv_fast_mem_valid_check(sq_desc, mem_src_ctrl, mem_dst_ctrl);
    if (ret != HDCDRV_OK) {
        hdcdrv_err("Calling hdcdrv_fast_mem_valid_check failed.\n");
        return HDCDRV_PARA_ERR;
    }

    return HDCDRV_OK;
}

void hdcdrv_node_msg_info_fill(u32 pid, u32 fid, int len, u64 addr,
    struct hdcdrv_fast_node_msg_info *msg)
{
    msg->pid = (u64)pid;
    msg->fid = fid;
    msg->len = (u32)len;
    msg->va_addr = addr;
    msg->hash_val = hdcdrv_get_hash(addr, (u64)pid, fid);
}

STATIC int hdcdrv_get_fast_mem_info(struct hdcdrv_sq_desc *sq_desc, struct hdcdrv_fast_mem_info *fmem_info)
{
    struct hdcdrv_dev_fmem *dev_fmem_lo = NULL;
    struct hdcdrv_dev_fmem *dev_fmem_re = NULL;
    int ret = HDCDRV_OK;
    int devid;
    u32 pid;
    u32 fid;
    struct hdcdrv_fast_node_msg_info node_msg;

    devid = hdc_ctrl->sessions[sq_desc->remote_session].dev_id;
    pid = (u32)hdc_ctrl->sessions[sq_desc->remote_session].owner_pid;
    fid = (hdc_ctrl->sessions[sq_desc->remote_session].owner == HDCDRV_SESSION_OWNER_VM) ?
        hdc_ctrl->sessions[sq_desc->remote_session].local_fid : 0;

    dev_fmem_lo = hdcdrv_get_dev_fmem_ex(devid, fid, HDCDRV_RBTREE_SIDE_LOCAL);
    dev_fmem_re = hdcdrv_get_dev_fmem_sep(devid);

    node_msg.dev_id = (u32)devid;
    if (sq_desc->src_data_addr != 0) {
        hdcdrv_node_msg_info_fill(sq_desc->src_pid, sq_desc->src_fid, sq_desc->data_len, sq_desc->src_data_addr,
            &node_msg);
        fmem_info->src_data = hdcdrv_get_fast_mem(dev_fmem_re, HDCDRV_FAST_MEM_TYPE_TX_DATA, &node_msg);
    }

    if (sq_desc->src_ctrl_addr != 0) {
        hdcdrv_node_msg_info_fill(sq_desc->src_pid, sq_desc->src_fid, sq_desc->ctrl_len, sq_desc->src_ctrl_addr,
            &node_msg);
        fmem_info->src_ctrl = hdcdrv_get_fast_mem(dev_fmem_re, HDCDRV_FAST_MEM_TYPE_TX_CTRL, &node_msg);
    }

    if (sq_desc->dst_data_addr != 0) {
        hdcdrv_node_msg_info_fill(pid, fid, sq_desc->data_len, sq_desc->dst_data_addr, &node_msg);
        fmem_info->dst_data = hdcdrv_get_fast_mem(dev_fmem_lo, HDCDRV_FAST_MEM_TYPE_RX_DATA, &node_msg);
    }

    if (sq_desc->dst_ctrl_addr != 0) {
        hdcdrv_node_msg_info_fill(pid, fid, sq_desc->ctrl_len, sq_desc->dst_ctrl_addr, &node_msg);
        fmem_info->dst_ctrl = hdcdrv_get_fast_mem(dev_fmem_lo, HDCDRV_FAST_MEM_TYPE_RX_CTRL, &node_msg);
    }

    if (hdcdrv_fast_mem_check(sq_desc, fmem_info->src_data, fmem_info->src_ctrl,
        fmem_info->dst_data, fmem_info->dst_ctrl) != HDCDRV_OK) {
        hdcdrv_set_node_to_idle(fmem_info);
        ret = HDCDRV_PARA_ERR;
    }

    return ret;
}

/* fast node unlock is in hdcdrv_rx_msg_callback function */
int hdcdrv_fast_dma_copy(struct hdcdrv_msg_chan *msg_chan, u32 head, struct hdcdrv_sq_desc *sq_desc,
    struct devdrv_asyn_dma_para_info *para, enum devdrv_dma_data_type data_type)
{
    struct hdcdrv_fast_mem_info fmem_info = {0};
    struct hdcdrv_buf_desc *rx_desc = &msg_chan->rx[head];
#if defined(CFG_FEATURE_VFIO) || defined(CFG_FEATURE_VFIO_DEVICE)
    struct hdcdrv_session *session = &hdc_ctrl->sessions[sq_desc->remote_session];
#endif
    int node_idx = 0;
    int ret;

    ret = hdcdrv_get_fast_mem_info(sq_desc, &fmem_info);
    if (ret != HDCDRV_OK) {
        hdcdrv_err("Calling hdcdrv_get_fast_mem_info failed.\n");
        return ret;
    }

    if (msg_chan->node == NULL) {
        msg_chan->node = kzalloc(HDCDRV_MAX_DMA_NODE * sizeof(struct devdrv_dma_node),
            GFP_ATOMIC | __GFP_NOWARN | __GFP_ACCOUNT);
        if (msg_chan->node == NULL) {
            hdcdrv_err("Alloc msg_chan->node failed. (dev=%u; size=%u)\n", msg_chan->dev_id,
                (u32)(HDCDRV_MAX_DMA_NODE * sizeof(struct devdrv_dma_node)));
            return HDCDRV_MEM_ALLOC_FAIL;
        }
    }

    rx_desc->dst_data_addr = sq_desc->dst_data_addr;
    rx_desc->len = sq_desc->data_len;
    rx_desc->dst_ctrl_addr = sq_desc->dst_ctrl_addr;
    rx_desc->ctrl_len = sq_desc->ctrl_len;
    rx_desc->local_session = sq_desc->remote_session;
    rx_desc->remote_session = sq_desc->local_session;

    rx_desc->src_data = fmem_info.src_data;
    rx_desc->dst_data = fmem_info.dst_data;
    rx_desc->src_ctrl = fmem_info.src_ctrl;
    rx_desc->dst_ctrl = fmem_info.dst_ctrl;

    if (hdcdrv_get_dma_node_info(fmem_info.src_data, fmem_info.dst_data, msg_chan, &node_idx,
        sq_desc->data_len) != HDCDRV_OK) {
        hdcdrv_err("Calling hdcdrv_get_dma_node_info failed. (dev_id=%u)\n", msg_chan->dev_id);
        ret = HDCDRV_MEM_NOT_MATCH;
        goto CHECK_ERR;
    }

    if (hdcdrv_get_dma_node_info(fmem_info.src_ctrl, fmem_info.dst_ctrl, msg_chan, &node_idx,
        sq_desc->ctrl_len) != HDCDRV_OK) {
        hdcdrv_err("Calling hdcdrv_get_dma_node_info failed. (dev_id=%u)\n", msg_chan->dev_id);
        ret = HDCDRV_MEM_NOT_MATCH;
        goto CHECK_ERR;
    }

#if defined(CFG_FEATURE_VFIO) || defined(CFG_FEATURE_VFIO_DEVICE)
    ret = hdcdrv_bandwidth_limit_check(session, hdcdrv_get_dma_direction(),
                                       (u32)(sq_desc->data_len + sq_desc->ctrl_len), (u32)node_idx);
    if (ret != HDCDRV_OK) {
        ret = HDCDRV_DMA_QUE_FULL;
        goto CHECK_ERR;
    }
#endif

    ret = devdrv_dma_async_link_copy_plus(msg_chan->dev_id, data_type, msg_chan->chan_id, msg_chan->node,
                                          (u32)node_idx, para);
    if (ret != HDCDRV_OK) {
        hdcdrv_err("Calling devdrv_dma_async_link_copy_plus failed. (dev_id=%u)\n", msg_chan->dev_id);
        ret = (ret == -ENOSPC) ? HDCDRV_DMA_QUE_FULL : HDCDRV_DMA_COPY_FAIL;
        goto CHECK_ERR;
    }

    return HDCDRV_OK;

CHECK_ERR:
    hdcdrv_set_node_to_idle(&fmem_info);

    msg_chan->rx[head].src_data = NULL;
    msg_chan->rx[head].dst_data = NULL;
    msg_chan->rx[head].src_ctrl = NULL;
    msg_chan->rx[head].dst_ctrl = NULL;

    return ret;
}

STATIC int hdcdrv_rx_msg_notify_task_handle(struct hdcdrv_msg_chan *msg_chan,
    struct hdcdrv_sq_desc *sq_desc, u32 head)
{
    struct devdrv_asyn_dma_para_info para;
    struct hdcdrv_session *session;
    long ret;

    para.interrupt_and_attr_flag = DEVDRV_LOCAL_IRQ_FLAG;
    para.priv = (void *)msg_chan;
    para.finish_notify = hdcdrv_rx_msg_callback;

    msg_chan->rx[head].local_session = sq_desc->remote_session;
    msg_chan->rx[head].remote_session = sq_desc->local_session;

    if ((ret = hdcdrv_session_inner_check(sq_desc->remote_session, sq_desc->inner_checker)) != HDCDRV_OK) {
        msg_chan->dbg_stat.hdcdrv_rx_msg_notify_task4++;

        hdcdrv_warn("Calling hdcdrv_session_inner_check not success. (dev=%u; session_id=%d)\n",
            msg_chan->dev_id, sq_desc->remote_session);
        msg_chan->rx[head].skip_flag = HDCDRV_VALID;
        goto JUMP_NEXT;
    }

    session = &hdc_ctrl->sessions[sq_desc->remote_session];
    // if normal, here equals msg->dbg_stat.hdcdrv_rx_msg_notify_task2++
    session->dbg_stat.hdcdrv_rx_msg_notify_task2++;
    msg_chan->rx[head].inner_checker = sq_desc->inner_checker;
    para.trans_id = head;

    if (msg_chan->type == HDCDRV_MSG_CHAN_TYPE_NORMAL) {
        ret = hdcdrv_normal_dma_copy(msg_chan, head, sq_desc, &para, msg_chan->data_type);
        if (ret == HDCDRV_DMA_MEM_ALLOC_FAIL) {
            /* wait schedule again when free mem */
            session->dbg_stat.hdcdrv_rx_msg_notify_task5++;
            msg_chan->dbg_stat.hdcdrv_rx_msg_notify_task5++;
            return HDCDRV_ERR;
        }
    } else {
        ret = hdcdrv_fast_dma_copy(msg_chan, head, sq_desc, &para, msg_chan->data_type);
    }

    if (ret == HDCDRV_DMA_QUE_FULL) {
        /* dma que full, wait sched again in the following cases:
           1. next send request in hdcdrv_rx_msg_notify
           2. dma completion interrupt in hdcdrv_rx_msg_callback
           3. received monitoring work in hdcdrv_rx_msg_notify_task_check  */
        session->dbg_stat.hdcdrv_rx_msg_notify_task6++;
        msg_chan->dbg_stat.hdcdrv_rx_msg_notify_task6++;
        msg_chan->dma_need_submit_flag = HDCDRV_VALID;
        return HDCDRV_ERR;
    } else if (ret != HDCDRV_OK) {
        session->dbg_stat.hdcdrv_rx_msg_notify_task7++;
        msg_chan->dbg_stat.hdcdrv_rx_msg_notify_task7++;
        hdcdrv_err("Dma copy failed. (ret=%ld; msg_chan_type=%d; device=%u; notify_head=%d; data_size=%x)\n",
            ret, msg_chan->type, msg_chan->dev_id, head, sq_desc->data_len);

        msg_chan->rx[head].skip_flag = HDCDRV_VALID;

        goto JUMP_NEXT;
    }

#ifdef CFG_FEATURE_MIRROR
    dma_sync_single_for_cpu(msg_chan->dev, msg_chan->rx[head].addr, (u32)msg_chan->rx[head].len, DMA_FROM_DEVICE);
#endif

    msg_chan->submit_dma_head = (int)head;
    msg_chan->rx[head].skip_flag = HDCDRV_INVALID;
    session->dbg_stat.hdcdrv_rx_msg_notify_task8++;
JUMP_NEXT:
    if (msg_chan->rx[head].skip_flag == HDCDRV_VALID) {
        if (msg_chan->submit_dma_head == msg_chan->dma_head) {
            msg_chan->dma_head = (int)head;
            msg_chan->submit_dma_head = (int)head;
            tasklet_schedule(&msg_chan->rx_task);
        }
    }

    return HDCDRV_OK;
}

STATIC void hdcdrv_rx_msg_notify_proc(struct hdcdrv_msg_chan *msg_chan)
{
    struct hdcdrv_sq_desc *sq_desc = NULL;
    u32 head = 0;
    int cnt = 0;
    long ret;

    msg_chan->dma_need_submit_flag = HDCDRV_INVALID;

    msg_chan->rx_notify_task_status.schedule_in++;
    msg_chan->dbg_stat.hdcdrv_rx_msg_notify_task1++;

    do {
        sq_desc = hdcdrv_get_r_sq_desc(msg_chan->chan, &head);
        if ((sq_desc == NULL) || (sq_desc->valid != HDCDRV_VALID)) {
            break;
        }

        rmb();

        msg_chan->dbg_stat.hdcdrv_rx_msg_notify_task2++;

        /* Reach the threshold and schedule out */
        if (cnt >= HDCDRV_RX_BUDGET) {
            msg_chan->dbg_stat.hdcdrv_rx_msg_notify_task3++;
            hdcdrv_rx_msg_schedule_task(msg_chan);
            break;
        }

        ret = hdcdrv_rx_msg_notify_task_handle(msg_chan, sq_desc, head);
        if (ret != HDCDRV_OK) {
            break;
        }

        sq_desc->remote_session = HDCDRV_SESSION_FD_INVALID;
        sq_desc->local_session = HDCDRV_SESSION_FD_INVALID;
        sq_desc->valid = HDCDRV_INVALID;
        msg_chan->dbg_stat.hdcdrv_rx_msg_notify_task8++;

        hdcdrv_move_r_sq_desc(msg_chan->chan);
        cnt++;
    } while (1);
}

STATIC void hdcdrv_rx_msg_notify_work(struct work_struct *p_work)
{
    struct hdcdrv_msg_chan *msg_chan = container_of(p_work, struct hdcdrv_msg_chan, rx_notify_work);
    hdcdrv_rx_msg_notify_proc(msg_chan);
}

STATIC void hdcdrv_rx_msg_notify_task(unsigned long data)
{
    struct hdcdrv_msg_chan *msg_chan = (struct hdcdrv_msg_chan *)((uintptr_t)data);
    hdcdrv_rx_msg_notify_proc(msg_chan);
}

void hdcdrv_rx_msg_notify(void *chan)
{
    struct hdcdrv_dev *dev = NULL;
    struct hdcdrv_msg_chan *msg_chan = hdcdrv_get_msg_chan_priv(chan);

    if (msg_chan == NULL) {
        hdcdrv_err("Input pararmeter is error.");
        return;
    }

    dev = &hdc_ctrl->devices[msg_chan->dev_id];

    msg_chan->dbg_stat.hdcdrv_rx_msg_notify1++;

    if (dev->valid != HDCDRV_VALID) {
        hdcdrv_err("Rx msg notify device reset. (dev_id=%u)\n", dev->dev_id);
        return;
    }

    hdcdrv_rx_msg_schedule_task(msg_chan);
}

STATIC void hdcdrv_rx_msg_notify_task_check(struct hdcdrv_msg_chan *msg_chan)
{
    struct hdcdrv_msg_chan_tasklet_status *tasklet_status = NULL;
    struct hdcdrv_sq_desc *sq_desc = NULL;
    u32 head = 0;

    tasklet_status = &msg_chan->rx_notify_task_status;

    msg_chan->dbg_stat.hdcdrv_rx_msg_notify_task_check1++;

    if (msg_chan->dma_need_submit_flag == HDCDRV_VALID) {
        hdcdrv_rx_msg_schedule_task(msg_chan);
        return;
    }

    if (hdcdrv_tasklet_status_check(tasklet_status) == HDCDRV_OK) {
        return;
    }

    sq_desc = hdcdrv_get_r_sq_desc(msg_chan->chan, &head);
    if (sq_desc == NULL) {
        return;
    }

    /* no valid msg */
    if (sq_desc->valid != HDCDRV_VALID) {
        return;
    }

    msg_chan->dbg_stat.hdcdrv_rx_msg_notify_task_check2++;

    /* there has msg but not schedule */
    hdcdrv_rx_msg_schedule_task(msg_chan);
}

STATIC void hdcdrv_clear_session_rx_buf(int session_fd)
{
    struct hdcdrv_session *session = NULL;
    struct hdcdrv_buf_desc *buf_desc = NULL;
    struct hdcdrv_msg_chan *msg_chan = NULL;
    struct hdcdrv_msg_chan *fast_msg_chan = NULL;

    /* Release the memory of a locally unclosed session */
    session = &hdc_ctrl->sessions[session_fd];

    if (session->chan_id < hdc_ctrl->devices[session->dev_id].normal_chan_num) {
        msg_chan = hdc_ctrl->devices[session->dev_id].msg_chan[session->chan_id];
    }

    if (session->fast_chan_id < (u32)hdc_ctrl->devices[session->dev_id].msg_chan_cnt) {
        fast_msg_chan = hdc_ctrl->devices[session->dev_id].msg_chan[session->fast_chan_id];
    }

    spin_lock_bh(&session->lock);
    while (session->normal_rx.head != session->normal_rx.tail) {
        buf_desc = &session->normal_rx.rx_list[session->normal_rx.head];
        session->normal_rx.head = (session->normal_rx.head + 1) % HDCDRV_SESSION_RX_LIST_MAX_PKT;
        hdcdrv_free_mem(session, buf_desc->buf, HDCDRV_MEMPOOL_FREE_IN_PM, buf_desc);
        hdcdrv_desc_clear(buf_desc);
    }

    session->normal_rx.head = 0;
    session->normal_rx.tail = 0;
    /* fast session clear */
    session->fast_rx.head = 0;
    session->fast_rx.tail = 0;

    if (msg_chan != NULL) {
        if (msg_chan->rx_trigger_flag == HDCDRV_VALID) {
            msg_chan->rx_trigger_flag = HDCDRV_INVALID;
            tasklet_schedule(&msg_chan->rx_task);
        }
    }

    if (fast_msg_chan != NULL) {
        if (fast_msg_chan->rx_trigger_flag == HDCDRV_VALID) {
            fast_msg_chan->rx_trigger_flag = HDCDRV_INVALID;
            tasklet_schedule(&fast_msg_chan->rx_task);
        }
    }
    spin_unlock_bh(&session->lock);
}

STATIC int hdcdrv_alloc_session(int service_type, int run_env, int root_privilege, u64 check_pid)
{
    int i, max, base, fd;
    int session_fd = -1;
    int conn_feature = hdcdrv_get_service_conn_feature(service_type);
    int *cur = NULL;

    mutex_lock(&hdc_ctrl->mutex);

AGAIN:
    if (conn_feature == HDCDRV_SERVICE_LONG_CONN) {
        max = (int)HDCDRV_REAL_MAX_LONG_SESSION;
        base = 0;
        cur = &hdc_ctrl->cur_alloc_session;
    } else {
        max = (int)HDCDRV_REAL_MAX_SHORT_SESSION;
        base = (int)HDCDRV_REAL_MAX_LONG_SESSION;
        cur = &hdc_ctrl->cur_alloc_short_session;
    }

    for (i = 0; i < max; i++) {
        fd = (*cur + i) % max + base;
        if ((hdc_ctrl->sessions[fd].create_pid != check_pid) &&
            (hdcdrv_get_session_status(&hdc_ctrl->sessions[fd]) == HDCDRV_SESSION_STATUS_IDLE)) {
                session_fd = fd;
                hdcdrv_set_session_status(&hdc_ctrl->sessions[fd], HDCDRV_SESSION_STATUS_CONN);
                *cur = (fd + 1) % max + base;
                break;
        }
    }

    if ((session_fd == -1) && (conn_feature == HDCDRV_SERVICE_SHORT_CONN) &&
        (run_env != HDCDRV_SESSION_RUN_ENV_PHYSICAL_CONTAINER) &&
        (run_env != HDCDRV_SESSION_RUN_ENV_VIRTUAL_CONTAINER) && root_privilege) {
        hdcdrv_info_limit("No session, try another. (service=\"%s\"; conn_feature=%d)",
            hdcdrv_sevice_str(service_type), conn_feature);
        conn_feature = HDCDRV_SERVICE_LONG_CONN;
        goto AGAIN;
    }

    mutex_unlock(&hdc_ctrl->mutex);

    if (session_fd < 0) {
        hdcdrv_print_status(service_type);
    }

    return session_fd;
}

u32 hdcdrv_alloc_fast_msg_chan(int dev_id, int service_type)
{
    struct hdcdrv_dev *dev = &hdc_ctrl->devices[dev_id];
    u32 chan_id = dev->normal_chan_num;
    int session_cnt;
    u32 i;

    mutex_lock(&dev->mutex);
    /* Find the msg chan with the least number of sessions */
    session_cnt = dev->msg_chan[dev->normal_chan_num]->session_cnt;
    for (i = dev->normal_chan_num; i < (u32)dev->msg_chan_cnt; i++) {
        if (session_cnt > dev->msg_chan[i]->session_cnt) {
            session_cnt = dev->msg_chan[i]->session_cnt;
            chan_id = i;
        }
    }
    mutex_unlock(&dev->mutex);

    return chan_id;
}

STATIC void hdcdrv_mod_msg_chan_session_cnt(int dev_id, u32 normal_chan_id, u32 fast_chan_id, int num)
{
    struct hdcdrv_dev *dev = &hdc_ctrl->devices[dev_id];

    if (normal_chan_id >= HDCDRV_SUPPORT_MAX_DEV_MSG_CHAN || fast_chan_id >= HDCDRV_SUPPORT_MAX_DEV_MSG_CHAN) {
        hdcdrv_warn("Normal or fast chan id is invalid. (dev=%d; normal=%d; fast=%d)\n",
            dev_id, normal_chan_id, fast_chan_id);
        return;
    }

    if ((dev->msg_chan[normal_chan_id] == NULL) || (dev->msg_chan[fast_chan_id] == NULL)) {
        hdcdrv_warn("Input pararmeter is is invalid. (dev=%d)\n", dev_id);
        return;
    }

    mutex_lock(&dev->mutex);
    dev->msg_chan[normal_chan_id]->session_cnt += num;
    dev->msg_chan[fast_chan_id]->session_cnt += num;
    mutex_unlock(&dev->mutex);
}

STATIC enum devdrv_dma_data_type hdcdrv_get_dma_data_type(int msg_chan_type, int chan_id)
{
    enum devdrv_dma_data_type data_type;

    /* Low-level use common type */
    if (msg_chan_type == HDCDRV_MSG_CHAN_TYPE_NORMAL) {
        if (hdcdrv_get_service_level(chan_id) == HDCDRV_SERVICE_HIGH_LEVEL) {
            data_type = DEVDRV_DMA_DATA_TRAFFIC;
        } else {
            data_type = DEVDRV_DMA_DATA_COMMON;
        }
    } else {
        data_type = DEVDRV_DMA_DATA_TRAFFIC;
    }

    return data_type;
}

STATIC void hdcdrv_session_stat_clear(struct hdcdrv_session *session)
{
    session->stat.rx = 0;
    session->stat.rx_bytes = 0;
    session->stat.rx_fail = 0;
    session->stat.rx_finish = 0;
    session->stat.rx_full = 0;
    session->stat.rx_total = 0;

    session->stat.tx = 0;
    session->stat.tx_bytes = 0;
    session->stat.tx_fail = 0;
    session->stat.tx_finish = 0;
    session->stat.tx_full = 0;
}

STATIC void hdcdrv_session_dbg_stat_clear(struct hdcdrv_session *session)
{
    session->dbg_stat.hdcdrv_mem_avail1 = 0;
    session->dbg_stat.hdcdrv_msg_chan_recv_task1 = 0;
    session->dbg_stat.hdcdrv_msg_chan_recv_task2 = 0;
    session->dbg_stat.hdcdrv_msg_chan_recv_task3 = 0;
    session->dbg_stat.hdcdrv_msg_chan_recv_task4 = 0;
    session->dbg_stat.hdcdrv_msg_chan_recv_task5 = 0;
    session->dbg_stat.hdcdrv_msg_chan_recv_task6 = 0;
    session->dbg_stat.hdcdrv_msg_chan_recv_task7 = 0;
    session->dbg_stat.hdcdrv_msg_chan_send1 = 0;
    session->dbg_stat.hdcdrv_normal_dma_copy1 = 0;
    session->dbg_stat.hdcdrv_recv_data_times = 0;
    session->dbg_stat.hdcdrv_rx_msg_callback1 = 0;
    session->dbg_stat.hdcdrv_rx_msg_callback2 = 0;
    session->dbg_stat.hdcdrv_rx_msg_callback3 = 0;
    session->dbg_stat.hdcdrv_rx_msg_notify1 = 0;
    session->dbg_stat.hdcdrv_rx_msg_notify_task1 = 0;
    session->dbg_stat.hdcdrv_rx_msg_notify_task2 = 0;
    session->dbg_stat.hdcdrv_rx_msg_notify_task3 = 0;
    session->dbg_stat.hdcdrv_rx_msg_notify_task4 = 0;
    session->dbg_stat.hdcdrv_rx_msg_notify_task5 = 0;
    session->dbg_stat.hdcdrv_rx_msg_notify_task6 = 0;
    session->dbg_stat.hdcdrv_rx_msg_notify_task7 = 0;
    session->dbg_stat.hdcdrv_rx_msg_notify_task8 = 0;
    session->dbg_stat.hdcdrv_rx_msg_notify_task_check1 = 0;
    session->dbg_stat.hdcdrv_rx_msg_notify_task_check2 = 0;
    session->dbg_stat.hdcdrv_rx_msg_task_check1 = 0;
    session->dbg_stat.hdcdrv_rx_msg_task_check2 = 0;
    session->dbg_stat.hdcdrv_tx_finish_notify1 = 0;
    session->dbg_stat.hdcdrv_tx_finish_notify_task1 = 0;
    session->dbg_stat.hdcdrv_tx_finish_notify_task2 = 0;
    session->dbg_stat.hdcdrv_tx_finish_notify_task3 = 0;
    session->dbg_stat.hdcdrv_tx_finish_notify_task4 = 0;
    session->dbg_stat.hdcdrv_tx_finish_notify_task5 = 0;
    session->dbg_stat.hdcdrv_tx_finish_notify_task6 = 0;
    session->dbg_stat.hdcdrv_tx_finish_notify_task7 = 0;
    session->dbg_stat.hdcdrv_tx_finish_task_check1 = 0;
    session->dbg_stat.hdcdrv_tx_finish_task_check2 = 0;
}

STATIC void hdcdrv_session_init(int local_session, int remote_session, int dev_id, int service_type, int run_env)
{
    struct hdcdrv_session *session = &hdc_ctrl->sessions[local_session];
    struct hdcdrv_dev *hdc_dev = &hdc_ctrl->devices[dev_id];

    hdcdrv_clear_session_rx_buf(local_session);
    /* after hdcdrv_sid_copy */
    hdcdrv_inner_checker_set(session);
    session->local_session_fd = local_session;
    session->remote_session_fd = remote_session;
    session->dev_id = dev_id;
    session->local_fid = HDCDRV_SESSION_OWNER_PM;
    session->remote_fid = HDCDRV_SESSION_OWNER_PM;
    session->service_type = service_type;
    session->service = &hdc_dev->service[service_type];
    session->run_env = run_env;
    session->timeout_jiffies.send_timeout = msecs_to_jiffies(HDCDRV_SESSION_DEFAULT_TIMEOUT);
    session->timeout_jiffies.fast_send_timeout = msecs_to_jiffies(HDCDRV_SESSION_DEFAULT_TIMEOUT);

    if (memset_s((void *)&session->stat, sizeof(session->stat), 0, sizeof(session->stat)) != EOK) {
        hdcdrv_err("Calling memset_s failed. (dev=%d)\n", dev_id);
        return;
    }

    session->owner_pid = HDCDRV_INVALID_PID;
    session->pid_flag = 0;
    session->delay_work_flag = 0;

    hdcdrv_session_stat_clear(session);
    hdcdrv_session_dbg_stat_clear(session);
}

STATIC int hdcdrv_session_chan_update(int local_session, int dev_id)
{
    struct hdcdrv_session *session = &hdc_ctrl->sessions[local_session];
    struct hdcdrv_dev *hdc_dev = &hdc_ctrl->devices[dev_id];

    if ((session->chan_id >= hdc_dev->normal_chan_num) || (session->fast_chan_id >= (u32)hdc_dev->msg_chan_cnt)) {
        return HDCDRV_PARA_ERR;
    }

    session->msg_chan = hdc_dev->msg_chan[session->chan_id];
    session->fast_msg_chan = hdc_dev->msg_chan[session->fast_chan_id];
    return HDCDRV_OK;
}

STATIC void hdcdrv_session_uid(int local_session, int current_euid, int current_uid, int current_root_privilege)
{
    struct hdcdrv_session *session = &hdc_ctrl->sessions[local_session];

    session->euid = current_euid;
    session->uid = current_uid;
    session->root_privilege = current_root_privilege;
}

STATIC void hdcdrv_free_service_conn_req(struct hdcdrv_service *service)
{
    struct hdcdrv_connect_list *connect_t = NULL;

    mutex_lock(&service->mutex);

    while (service->conn_list_head != NULL) {
        connect_t = service->conn_list_head;
        service->conn_list_head = service->conn_list_head->next;
        kfree(connect_t);
        connect_t = NULL;
    }

    mutex_unlock(&service->mutex);
}

#ifdef HDC_ENV_DEVICE
STATIC bool hdcdrv_check_run_env_is_phy(u32 dev_id)
{
    u32 host_flag = 0;
    bool is_in = false;
    int ret;
    int vf_flag = DEVDRV_SRIOV_TYPE_PF;

#ifdef CFG_FEATURE_SRIOV
    vf_flag = devdrv_get_pfvf_type_by_devid(dev_id);
#endif

    ret = agentdrv_get_host_phy_mach_flag(dev_id, &host_flag);
    if ((ret == 0) && (host_flag == DEVDRV_HOST_PHY_MACH_FLAG) && (vf_flag == DEVDRV_SRIOV_TYPE_PF)) {
        is_in = true;
    }

    return is_in;
}
#endif

STATIC int hdcdrv_ctrl_msg_connect_handle(const struct hdcdrv_ctrl_msg *msg, struct hdcdrv_service *service,
    u32 devid)
{
    struct hdcdrv_session *session = NULL;
    struct hdcdrv_connect_list *connect_n = NULL;
    struct hdcdrv_connect_list *connect_t = NULL;
    int session_fd, run_env;
    int service_type = msg->connect_msg.service_type;
    u32 normal_chan_id;
    u32 fast_chan_id;
    u64 listen_pid = service->listen_pid;
    int ret;

#ifdef HDC_ENV_DEVICE
    if ((msg->connect_msg.run_env == HDCDRV_SESSION_RUN_ENV_PHYSICAL) && (!hdcdrv_check_run_env_is_phy(devid))) {
        hdcdrv_err_limit("Permission not correct, connect failed.(dev_id=%u)\n", devid);
        return HDCDRV_NO_PERMISSION;
    }
#endif

    connect_n = (struct hdcdrv_connect_list *)kzalloc(sizeof(struct hdcdrv_connect_list),
        GFP_KERNEL | __GFP_ACCOUNT);
    if (unlikely(connect_n == NULL)) {
        hdcdrv_err("Alloc connect_t failed. (dev=%u; client_session=%d)\n", devid, msg->connect_msg.client_session);
        return HDCDRV_MEM_ALLOC_FAIL;
    }

    ret = hdcdrv_session_pre_alloc(devid, msg->connect_msg.fid, service_type);
    if (ret != HDCDRV_OK) {
        kfree(connect_n);
        connect_n = NULL;
        hdcdrv_err("Calling hdcdrv_session_pre_alloc failed. (dev_id=%d; fid=%d; service_type=\"%s\"; ret=%d)\n",
            devid, msg->connect_msg.fid, hdcdrv_sevice_str(service_type), ret);
        return ret;
    }

    session_fd = hdcdrv_alloc_session(service_type, msg->connect_msg.run_env,
        msg->connect_msg.root_privilege, listen_pid);
    if (session_fd < 0) {
        kfree(connect_n);
        connect_n = NULL;
        hdcdrv_err("Calling hdcdrv_alloc_session failed. (dev_id=%u; service_type=\"%s\"; listen_pid=%llu)\n",
            devid, hdcdrv_sevice_str(service_type), listen_pid);
        hdcdrv_session_post_free(devid, msg->connect_msg.fid, service_type);
        return HDCDRV_NO_SESSION;
    }

    session = &hdc_ctrl->sessions[session_fd];
    session->unique_val = msg->connect_msg.unique_val;

    if ((msg->connect_msg.fast_chan_id != HDCDRV_INVALID_VALUE) &&
        (msg->connect_msg.normal_chan_id != HDCDRV_INVALID_VALUE)) {
        fast_chan_id = msg->connect_msg.fast_chan_id;
        normal_chan_id = msg->connect_msg.normal_chan_id;
    } else {
        hdcdrv_alloc_session_chan((int)devid, (int)msg->connect_msg.fid, service_type, &normal_chan_id, &fast_chan_id);
    }

    run_env = msg->connect_msg.run_env;

    hdcdrv_session_uid(session_fd, msg->connect_msg.euid, msg->connect_msg.uid, msg->connect_msg.root_privilege);
    hdcdrv_session_init(session_fd, msg->connect_msg.client_session, (int)devid, service_type, run_env);

    session->remote_fid = msg->connect_msg.fid;
    session->peer_create_pid = msg->connect_msg.local_pid;
    session->chan_id = normal_chan_id;
    session->fast_chan_id = fast_chan_id;

    ret = hdcdrv_session_chan_update(session_fd, (int)devid);
    if (ret != HDCDRV_OK) {
        kfree(connect_n);
        connect_n = NULL;
        hdcdrv_set_session_status(session, HDCDRV_SESSION_STATUS_IDLE);
        hdcdrv_session_post_free(devid, msg->connect_msg.fid, service_type);
        hdcdrv_err("Calling hdcdrv_session_chan_update error. (device=%d; service=\"%s\"; "
            "error_ret=%d; local_session_fd=%x; remote_session_fd=%x; pid=%llu)\n",
            devid, hdcdrv_sevice_str(service_type), (int)ret, session->local_session_fd,
            session->remote_session_fd, session->owner_pid);
        return ret;
    }
    hdcdrv_mod_msg_chan_session_cnt((int)devid, session->chan_id, session->fast_chan_id, 1);

    connect_n->session_fd = session_fd;
    connect_n->next = NULL;

    /* wakeup accept thread */
    mutex_lock(&service->mutex);
    if (service->conn_list_head == NULL) {
        service->conn_list_head = connect_n;
    } else {
        connect_t = service->conn_list_head;
        while (connect_t->next != NULL) {
            connect_t = connect_t->next;
        }
        connect_t->next = connect_n;
    }
    mutex_unlock(&service->mutex);

    (void)hdcdrv_connect_notify(service_type, (int)devid, msg->connect_msg.fid,
        hdcdrv_rebuild_raw_pid(msg->connect_msg.local_pid), hdcdrv_rebuild_raw_pid(service->listen_pid));
    hdcdrv_delay_work_set(session, &session->close_unknow_session, HDCDRV_DELAY_UNKNOWN_SESSION_BIT,
        HDCDRV_SESSION_RECLAIM_TIMEOUT);
    wmb();

    wake_up_interruptible(&service->wq_conn_avail);
    hdcdrv_epoll_wake_up(service->epfd);

    return HDCDRV_OK;
}

STATIC int hdcdrv_ctrl_msg_connect(u32 devid, struct hdcdrv_ctrl_msg *msg)
{
    struct hdcdrv_dev *hdc_dev = NULL;
    struct hdcdrv_service *service = NULL;
    int service_type = msg->connect_msg.service_type;
    u32 fast_chan_id = msg->connect_msg.fast_chan_id;
    u32 normal_chan_id = msg->connect_msg.normal_chan_id;
    u64 remote_pid = msg->connect_msg.local_pid;
    u64 peer_pid = msg->connect_msg.peer_pid;
    u32 fid = msg->connect_msg.fid;
    u64 create_pid;
    int ret;

    hdc_dev = &hdc_ctrl->devices[devid];

    /* in virtual case msg chan_id maybe equal HDCDRV_INVALID_CHAN_ID */
    if (((fast_chan_id >= (u32)hdc_dev->msg_chan_cnt) && (fast_chan_id != HDCDRV_INVALID_CHAN_ID)) ||
        ((normal_chan_id >= hdc_dev->normal_chan_num) && (normal_chan_id != HDCDRV_INVALID_CHAN_ID))) {
        hdcdrv_err("Input pararmeter is error. (dev_id=%u; fast_chan_id=%d; normal_chan_id=%d)\n",
            devid, fast_chan_id, normal_chan_id);
        return HDCDRV_PARA_ERR;
    }

    if ((service_type >= HDCDRV_SUPPORT_MAX_SERVICE) || (service_type < 0)) {
        hdcdrv_err_limit("service_type is error. (dev_id=%u; service_type=\"%s\")\n",
            devid, hdcdrv_sevice_str(service_type));
        return HDCDRV_PARA_ERR;
    }

    peer_pid = hdcdrv_get_peer_pid(devid, remote_pid, fid, peer_pid, service_type);
    create_pid = hdcdrv_rebuild_pid(devid, fid, peer_pid);
    service = hdcdrv_search_service(devid, fid, service_type, create_pid);
    if (service->listen_status == HDCDRV_INVALID) {
        /* send to remote to info local service not listen. */
        return HDCDRV_REMOTE_SERVICE_NO_LISTENING;
    }

    ret = hdcdrv_ctrl_msg_connect_handle(msg, service, devid);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    hdcdrv_link_ctrl_msg_stats_add(devid, service_type, HDCDRV_CTRL_MSG_TYPE_CONNECT,
        HDCDRV_LINK_CTRL_MSG_RECV_SUCC, HDCDRV_OK);
    return HDCDRV_OK;
}

STATIC int hdcdrv_ctrl_msg_connect_reply(u32 devid, const struct hdcdrv_ctrl_msg *msg)
{
    struct hdcdrv_session *session = NULL;
    int client_session = msg->connect_msg_reply.client_session;
    int server_session = msg->connect_msg_reply.server_session;
    u32 unique_val = msg->connect_msg_reply.unique_val;
    int ret;

    ret = (int)hdcdrv_session_alive_check(client_session, (int)devid, unique_val);
    if (ret != HDCDRV_OK) {
        /* if the session is closed, need to return exception code. */
        return ret;
    }

    session = &hdc_ctrl->sessions[client_session];

#ifdef HDC_ENV_DEVICE
    if ((msg->connect_msg_reply.run_env == HDCDRV_SESSION_RUN_ENV_PHYSICAL) && (!hdcdrv_check_run_env_is_phy(devid))) {
        hdcdrv_err_limit("Permission not correct, connect failed.(dev_id=%u)\n", devid);
        return HDCDRV_NO_PERMISSION;
    }
#endif

    session->run_env = msg->connect_msg_reply.run_env;
    session->remote_fid = msg->connect_msg_reply.fid;
    session->peer_create_pid = msg->connect_msg_reply.local_pid;

    if ((session->fast_chan_id == HDCDRV_INVALID_VALUE) &&
        (session->chan_id == HDCDRV_INVALID_VALUE)) {
        session->fast_chan_id = msg->connect_msg_reply.fast_chan_id;
        session->chan_id = msg->connect_msg_reply.normal_chan_id;
    }

    ret = hdcdrv_session_chan_update(client_session, (int)devid);
    if (ret == HDCDRV_OK) {
        session->remote_session_fd = server_session;
    } else {
        session->remote_session_fd = HDCDRV_INVALID_REMOTE_SESSION_ID;
    }
    wmb();

    wake_up_interruptible(&session->wq_conn);
    hdcdrv_link_ctrl_msg_stats_add(devid, session->service_type, HDCDRV_CTRL_MSG_TYPE_CONNECT_REPLY,
        HDCDRV_LINK_CTRL_MSG_RECV_SUCC, HDCDRV_OK);

    if (hdcdrv_get_session_status(session) != HDCDRV_SESSION_STATUS_CONN) {
        return HDCDRV_SESSION_HAS_CLOSED;
    }

    return ret;
}

STATIC void hdcdrv_remote_close_work(struct work_struct *p_work)
{
    long ret;
    u32 delay;
    struct hdcdrv_cmd_close close_cmd;
    struct hdcdrv_session *session = container_of(p_work, struct hdcdrv_session, remote_close.work);

    hdcdrv_delay_work_flag_clear(session, HDCDRV_DELAY_REMOTE_CLOSE_BIT);

    delay = jiffies_to_msecs(jiffies - session->remote_close_jiff);
    if (delay < HDCDRV_SESSION_REMOTE_CLOSED_TIMEOUT_MS &&
        ((session->remote_close_state == HDCDRV_CLOSE_TYPE_USER) ||
        (session->remote_close_state == HDCDRV_CLOSE_TYPE_KERNEL))) {
        msleep(HDCDRV_SESSION_REMOTE_CLOSED_TIMEOUT_MS - delay);
    }

    if (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_REMOTE_CLOSED) {
        close_cmd.session = session->local_session_fd;
        close_cmd.pid = session->owner_pid;
        close_cmd.unique_val = session->unique_val;

        ret = hdcdrv_close(&close_cmd, HDCDRV_CLOSE_TYPE_REMOTE_CLOSED_POST);
        if (ret != HDCDRV_OK) {
            hdcdrv_warn_limit("Close session not success. (devid=%d; chanid=%d; sessionid=%d)\n",
                session->dev_id, session->chan_id, session->local_session_fd);
        }
    }
}

STATIC void hdcdrv_wake_up_context_status(const struct hdcdrv_session *session)
{
#ifndef CFG_DISABLE_TSD_NOTIFY
    read_lock(&g_symbol_lock);
    if ((g_hdcdrv_register_symbol.module_ptr != NULL) && (session->owner_pid != HDCDRV_INVALID_PID) &&
        (session->service_type == HDCDRV_SERVICE_TYPE_TSD)) {
        __module_get(g_hdcdrv_register_symbol.module_ptr);
        (void)g_hdcdrv_register_symbol.wake_up_context_status((int)session->owner_pid, (u32)session->dev_id,
            DEVDRV_STATUS_HDC_CLOSE_FLAG);
        module_put(g_hdcdrv_register_symbol.module_ptr);
    }
    read_unlock(&g_symbol_lock);
#endif
    return;
}

STATIC int hdcdrv_ctrl_msg_close(u32 devid, const struct hdcdrv_ctrl_msg *msg)
{
    struct hdcdrv_session *session = NULL;
    int session_fd = msg->close_msg.remote_session;
    long ret;
    int timeout;

    if ((session_fd >= HDCDRV_REAL_MAX_SESSION) || (session_fd < 0)) {
        hdcdrv_err("session_fd is illegal. (session_fd=%d)\n", session_fd);
        return HDCDRV_PARA_ERR;
    }

    session = &hdc_ctrl->sessions[session_fd];

    ret = hdcdrv_session_state_to_remote_close(session_fd, msg->close_msg.unique_val);
    if (ret != HDCDRV_OK) {
        if (ret == HDCDRV_SESSION_HAS_CLOSED) {
            hdcdrv_link_ctrl_msg_stats_add(devid, session->service_type, HDCDRV_CTRL_MSG_TYPE_CLOSE,
                HDCDRV_LINK_CTRL_MSG_RECV_SUCC, (int)ret);
            return HDCDRV_OK;
        } else {
            hdcdrv_link_ctrl_msg_stats_add(devid, session->service_type, HDCDRV_CTRL_MSG_TYPE_CLOSE,
                HDCDRV_LINK_CTRL_MSG_RECV_FAIL, (int)ret);
            return (int)ret;
        }
    }

    (void)hdcdrv_close_notify(session->service_type, (int)devid, (int)session->remote_fid,
        hdcdrv_rebuild_raw_pid(session->peer_create_pid), hdcdrv_rebuild_raw_pid(session->owner_pid));

    session->local_close_state = HDCDRV_CLOSE_TYPE_REMOTE_CLOSED_PRE;
    session->remote_close_state = msg->close_msg.session_close_state;
    timeout = ((session->remote_close_state == HDCDRV_CLOSE_TYPE_USER) ||
        (session->remote_close_state == HDCDRV_CLOSE_TYPE_KERNEL)) ? HDCDRV_SESSION_REMOTE_CLOSED_TIMEOUT : 0;
    session->remote_close_jiff = jiffies;

    hdcdrv_delay_work_set(session, &session->remote_close, HDCDRV_DELAY_REMOTE_CLOSE_BIT, timeout);
    hdcdrv_wake_up_context_status(session);

    hdcdrv_link_ctrl_msg_stats_add(devid, session->service_type, HDCDRV_CTRL_MSG_TYPE_CLOSE,
        HDCDRV_LINK_CTRL_MSG_RECV_SUCC, HDCDRV_OK);

    return HDCDRV_OK;
}

void hdcdrv_set_device_status(int devid, u32 valid)
{
    struct hdcdrv_dev *dev = &hdc_ctrl->devices[devid];
    dev->valid = valid;
    hdcdrv_info("Set status finished.(devid=%d; status=%d)\n", devid, valid);
}

int hdcdrv_get_device_status(int devid)
{
    struct hdcdrv_dev *dev = &hdc_ctrl->devices[devid];
    return (int)dev->valid;
}

STATIC void hdcdrv_set_device_reset(u32 devid)
{
    hdcdrv_del_dev(devid);
}

void hdcdrv_set_device_para(u32 devid, int normal_chan_num)
{
    struct hdcdrv_dev *dev = &hdc_ctrl->devices[devid];
    if (normal_chan_num > 0) {
        dev->normal_chan_num = (u32)normal_chan_num;
    }
    hdcdrv_info("Set normal chan number ok.(dev_id=%d; normal_chan_num=%d)\n", devid, normal_chan_num);
}

STATIC int hdcdrv_ctrl_msg_sync(u32 devid, struct hdcdrv_ctrl_msg *msg)
{
    int segment = msg->sync_msg.segment;
    int peer_dev_id = msg->sync_msg.peer_dev_id;

    if ((segment < 0) || (segment > HDCDRV_HUGE_PACKET_SEGMENT)) {
        hdcdrv_err("segment is invalid. (dev_id=%d; segment=%d)\n", devid, segment);
        return HDCDRV_PARA_ERR;
    }
    if (hdc_ctrl->devices[devid].dev == NULL) {
        hdcdrv_warn("Device is not init instance. (dev_id=%d)\n", devid);
        return -EUNATCH;
    }

    hdcdrv_set_segment(segment);
    hdcdrv_set_peer_dev_id((int)devid, peer_dev_id);

    /* notice host device devid */
    msg->sync_msg.peer_dev_id = (int)devid;
    hdcdrv_info("Calling hdcdrv_ctrl_msg_sync succeeded.(dev_id=%d; peer_dev_id=%d)\n", devid, peer_dev_id);
    return HDCDRV_OK;
}

STATIC int hdcdrv_ctrl_edge_check(u32 devid, void *data, u32 in_data_len, u32 out_data_len, const u32 *p_real_out_len)
{
    struct hdcdrv_ctrl_msg *msg = (struct hdcdrv_ctrl_msg *)data;
    struct hdcdrv_ctrl_msg_sync_mem_info *mem = NULL;
    u32 in_len_min = 0;
    u32 out_len_min = 0;

    /* Here check 3 structures: hdcdrv_ctrl_msg_sync_mem_info, hdcdrv_sysfs_ctrl_msg, hdcdrv_ctrl_msg.
        The first element of the three structures is int, indicating the type of the msg.
        For security purposes, ensure that the length of in_data_len exceeds sizeof(int) before using msg->type. */
    if ((devid >= (u32)hdcdrv_get_max_support_dev()) || (msg == NULL) || (p_real_out_len == NULL)
        || (in_data_len < sizeof(int))) {
        hdcdrv_err("Input pararmeter is error.\n");
        return HDCDRV_PARA_ERR;
    }

    if (msg->type == HDCDRV_CTRL_MSG_TYPE_SYNC_MEM_INFO) {
        if (in_data_len < sizeof(struct hdcdrv_ctrl_msg_sync_mem_info)) {
            hdcdrv_err("in_data_len is smaller than expected (dev_id=%d)\n", devid);
            return HDCDRV_PARA_ERR;
        }
        mem = (struct hdcdrv_ctrl_msg_sync_mem_info *)data;
        if ((u32)mem->phy_addr_num > HDCDRV_MEM_MAX_PHY_NUM) {
            hdcdrv_err("phy_addr_num is biger than expected (dev_id=%d; phy_addr_num=%d)\n", devid, mem->phy_addr_num);
            return HDCDRV_PARA_ERR;
        }
        in_len_min = (u32)sizeof(struct hdcdrv_ctrl_msg_sync_mem_info) +
            mem->phy_addr_num * sizeof(struct hdcdrv_dma_mem);
        out_len_min = (u32)sizeof(struct hdcdrv_ctrl_msg_sync_mem_info);
    } else if ((msg->type == HDCDRV_CTRL_MSG_TYPE_GET_DEV_SESSION_STAT) ||
        (msg->type == HDCDRV_CTRL_MSG_TYPE_GET_DEV_CHAN_STAT) ||
        (msg->type == HDCDRV_CTRL_MSG_TYPE_GET_DEV_LINK_STAT)) {
        in_len_min = (u32)sizeof(struct hdcdrv_sysfs_ctrl_msg) + HDCDRV_SYSFS_DATA_MAX_LEN;
        out_len_min = (u32)sizeof(struct hdcdrv_sysfs_ctrl_msg);
    } else {
        in_len_min = (u32)sizeof(struct hdcdrv_ctrl_msg);
        out_len_min = (u32)sizeof(struct hdcdrv_ctrl_msg);
    }

    if ((in_data_len < in_len_min) || (out_data_len < out_len_min)) {
        hdcdrv_err("Input pararmeter is error. (dev=%d; data_in_len=%d; min_size=%d; out_len=%d, min_size=%d)\n",
            devid, in_data_len, in_len_min, out_data_len, out_len_min);
        return HDCDRV_PARA_ERR;
    }

    return HDCDRV_OK;
}

STATIC void hdcdrv_ctrl_msg_recv_exception_report(u32 devid, const struct hdcdrv_ctrl_msg *msg)
{
    if ((msg->error_code != HDCDRV_OK) && (msg->error_code != -EUNATCH) &&
        (msg->type != HDCDRV_CTRL_MSG_TYPE_CONNECT) && (msg->type != HDCDRV_CTRL_MSG_TYPE_CLOSE)) {
        hdcdrv_err_limit("Command executed Failed. (dev=%d; cmd=%d; error_code=%d)\n",
            devid, msg->type, msg->error_code);
    }
}

int hdcdrv_ctrl_msg_recv(u32 devid, void *data, u32 in_data_len, u32 out_data_len, u32 *real_out_len)
{
    struct hdcdrv_ctrl_msg *msg = (struct hdcdrv_ctrl_msg *)data;

    if (hdcdrv_ctrl_edge_check(devid, data, in_data_len, out_data_len, real_out_len) != HDCDRV_OK) {
        hdcdrv_err("Input parameters check failed. (dev=%d)\n", devid);
        return HDCDRV_PARA_ERR;
    }

    msg->error_code = HDCDRV_OK;
    *real_out_len = (sizeof(struct hdcdrv_ctrl_msg));

    switch (msg->type) {
        case HDCDRV_CTRL_MSG_TYPE_CONNECT:
            msg->error_code = hdcdrv_ctrl_msg_connect(devid, msg);
            break;
        case HDCDRV_CTRL_MSG_TYPE_CONNECT_REPLY:
            msg->error_code = hdcdrv_ctrl_msg_connect_reply(devid, msg);
            break;
        case HDCDRV_CTRL_MSG_TYPE_CLOSE:
            msg->error_code = hdcdrv_ctrl_msg_close(devid, msg);
            break;
        case HDCDRV_CTRL_MSG_TYPE_SYNC:
            msg->error_code = hdcdrv_ctrl_msg_sync(devid, msg);
            break;
        case HDCDRV_CTRL_MSG_TYPE_RESET:
            hdcdrv_set_device_reset(devid);
            break;
        case HDCDRV_CTRL_MSG_TYPE_CHAN_SET:
            hdcdrv_set_device_para(devid, msg->chan_set_msg.normal_chan_num);
            break;
        case HDCDRV_CTRL_MSG_TYPE_SYNC_MEM_INFO:
            *real_out_len = sizeof(struct hdcdrv_ctrl_msg_sync_mem_info);
            msg->error_code = hdcdrv_set_mem_info((int)devid, 0, HDCDRV_RBTREE_SIDE_REMOTE,
                (struct hdcdrv_ctrl_msg_sync_mem_info *)data);
            break;
        case HDCDRV_CTRL_MSG_TYPE_GET_DEV_SESSION_STAT:
            msg->error_code = hdcdrv_sysfs_ctrl_msg_get_session_stat(devid, data, real_out_len);
            break;
        case HDCDRV_CTRL_MSG_TYPE_GET_DEV_CHAN_STAT:
            msg->error_code = hdcdrv_sysfs_ctrl_msg_get_chan_stat(devid, data, real_out_len);
            break;
        case HDCDRV_CTRL_MSG_TYPE_GET_DEV_LINK_STAT:
            msg->error_code = hdcdrv_sysfs_ctrl_msg_get_connect_stat(devid, data, real_out_len);
            break;
        default:
            hdcdrv_err_limit("Device command is illegal. (dev=%d; cmd=%d)\n", devid, msg->type);
            return HDCDRV_PARA_ERR;
    }

    hdcdrv_ctrl_msg_recv_exception_report(devid, msg);

    return HDCDRV_OK;
}

long hdcdrv_dev_para_check(int dev_id, int service_type)
{
    if ((dev_id >= hdcdrv_get_max_support_dev()) || (dev_id < 0)) {
        hdcdrv_err_limit("Input pararmeter is error. (dev_id=%d)\n", dev_id);
        return HDCDRV_PARA_ERR;
    }

    if ((service_type >= HDCDRV_SUPPORT_MAX_SERVICE) || (service_type < 0)) {
        hdcdrv_err_limit("Input pararmeter is error. (dev_id=%d; service_type=%d)\n", dev_id, service_type);
        return HDCDRV_PARA_ERR;
    }

    if (hdc_ctrl->devices[dev_id].valid != HDCDRV_VALID) {
        return HDCDRV_DEVICE_NOT_READY;
    }

    return HDCDRV_OK;
}

STATIC long hdcdrv_cmd_get_peer_dev_id(struct hdcdrv_cmd_get_peer_dev_id *cmd)
{
    long ret;

    /* reuse dev check func with default service type */
    ret = hdcdrv_dev_para_check(cmd->dev_id, 0);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    cmd->peer_dev_id = hdcdrv_get_peer_dev_id(cmd->dev_id);

    return HDCDRV_OK;
}

STATIC long hdcdrv_config(struct hdcdrv_cmd_config *cmd)
{
    if (hdc_ctrl->segment == HDCDRV_INVALID_PACKET_SEGMENT) {
        return HDCDRV_PARA_ERR;
    }

    if (cmd->segment > hdcdrv_mem_block_capacity()) {
        cmd->segment = hdcdrv_mem_block_capacity();
    }

    return HDCDRV_OK;
}

STATIC long hdcdrv_set_service_level(const struct hdcdrv_cmd_set_service_level *cmd)
{
    int service_type = cmd->service_type;

    if ((service_type >= HDCDRV_SUPPORT_MAX_SERVICE) || (service_type < 0)) {
        hdcdrv_err("Input pararmeter is error. (service_type=%d)\n", service_type);
        return HDCDRV_PARA_ERR;
    }

    if ((cmd->level != HDCDRV_SERVICE_HIGH_LEVEL) && (cmd->level != HDCDRV_SERVICE_LOW_LEVEL)) {
        hdcdrv_err("service level is illegal. (level=%d)\n", cmd->level);
        return HDCDRV_PARA_ERR;
    }

    return HDCDRV_OK;
}

STATIC long hdcdrv_server_create(struct hdcdrv_ctx *ctx, const struct hdcdrv_cmd_server_create *cmd, u32 fid)
{
    struct hdcdrv_service *service = NULL;
    long ret;
    u32 create_fid;

    if (ctx == NULL) {
        hdcdrv_err("Input pararmeter is error. (dev_id=%d; service_type=\"%s\")\n",
            cmd->dev_id, hdcdrv_sevice_str(cmd->service_type));
        return HDCDRV_PARA_ERR;
    }

    ret = hdcdrv_dev_para_check(cmd->dev_id, cmd->service_type);
    if (ret != 0) {
        return ret;
    }

    create_fid = fid;
#ifdef CFG_FEATURE_VFIO_DEVICE
    if (cmd->service_type == HDCDRV_SERVICE_TYPE_TDT) {
        create_fid = hdcdrv_get_fid(hdcdrv_get_pid());
    }
#endif

    service = hdcdrv_alloc_service((u32)cmd->dev_id, create_fid, cmd->service_type, cmd->pid);
    if (service == NULL) {
        hdcdrv_err("Cann't create server. (dev_id=%d; service_type=\"%s\")\n", cmd->dev_id,
            hdcdrv_sevice_str(cmd->service_type));
        return HDCDRV_MEM_ALLOC_FAIL;
    }

    mutex_lock(&service->mutex);

    if (service->listen_status == HDCDRV_VALID) {
        mutex_unlock(&service->mutex);
        return HDCDRV_SERVICE_LISTENING;
    }

    service->listen_pid = cmd->pid;
    service->listen_status = HDCDRV_VALID;
    service->fid = create_fid;
    if (memset_s((void *)&service->data_stat, sizeof(service->data_stat), 0, sizeof(service->data_stat)) != EOK) {
        service->listen_pid = 0;
        service->listen_status = HDCDRV_INVALID;
        mutex_unlock(&service->mutex);
        hdcdrv_free_service(service);
        hdcdrv_err("Calling memset_s failed.\n");
        return HDCDRV_SAFE_MEM_OP_FAIL;
    }

    hdcdrv_bind_server_ctx(ctx, service, cmd->dev_id, cmd->service_type);

    mutex_unlock(&service->mutex);

    hdcdrv_info("Server create success.(dev_id=%d; fid=%u; service_type=\"%s\"; pid=%llu)\n",
        cmd->dev_id, fid, hdcdrv_sevice_str(cmd->service_type), cmd->pid);

    return HDCDRV_OK;
}

STATIC long hdcdrv_server_para_check(int dev_id, int service_type)
{
    if ((dev_id >= hdcdrv_get_max_support_dev()) || (dev_id < 0)) {
        hdcdrv_err("Input pararmeter is error. (dev_id=%d)\n", dev_id);
        return HDCDRV_PARA_ERR;
    }

    if ((service_type >= HDCDRV_SUPPORT_MAX_SERVICE)  || (service_type < 0)) {
        hdcdrv_err("Input pararmeter is error. (service_type=%d)\n", service_type);
        return HDCDRV_PARA_ERR;
    }

    return HDCDRV_OK;
}

long hdcdrv_server_free(struct hdcdrv_service *service, int dev_id, int service_type)
{
    mutex_lock(&service->mutex);

    if (service->listen_status == HDCDRV_INVALID) {
        mutex_unlock(&service->mutex);
        hdcdrv_warn("Server has not been created. (dev_id=%d; service_type=\"%s\")\n",
            dev_id, hdcdrv_sevice_str(service_type));
        return HDCDRV_SERVICE_NO_LISTENING;
    }

    hdcdrv_unbind_server_ctx(service);
    service->listen_status = HDCDRV_INVALID;

    mutex_unlock(&service->mutex);

    wake_up_interruptible(&service->wq_conn_avail);
    hdcdrv_free_service_conn_req(service);
    hdcdrv_free_service(service);

    return HDCDRV_OK;
}

STATIC long hdcdrv_server_destroy(const struct hdcdrv_ctx *ctx, const struct hdcdrv_cmd_server_destroy *cmd, u32 fid)
{
    struct hdcdrv_service *service = NULL;
    long ret;
    u32 create_fid;

    ret = hdcdrv_server_para_check(cmd->dev_id, cmd->service_type);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    create_fid = fid;
#ifdef CFG_FEATURE_VFIO_DEVICE
    if (cmd->service_type == HDCDRV_SERVICE_TYPE_TDT) {
        create_fid = hdcdrv_get_fid(cmd->pid);
    }
#endif

    service = hdcdrv_search_service((u32)cmd->dev_id, create_fid, cmd->service_type, cmd->pid);
    if (service->listen_pid != cmd->pid) {
        hdcdrv_warn("Server has already been destroyed. (dev_id=%d; service=\"%s\"; pid=%llu; owner_pid=%llu)\n",
            cmd->dev_id, hdcdrv_sevice_str(cmd->service_type), cmd->pid, service->listen_pid);
        return HDCDRV_OK;
    }

    if ((service->ctx != ctx) && (ctx != HDCDRV_KERNEL_WITHOUT_CTX)) {
        hdcdrv_err("service_ctx not match. (dev_id=%d; fid=%u; service_type=\"%s\")\n",
            cmd->dev_id, fid, hdcdrv_sevice_str(cmd->service_type));
        return HDCDRV_PARA_ERR;
    }

    hdcdrv_info("Server destroy success.(dev_id=%d; fid=%u; service=\"%s\"; pid=%llu)\n",
        cmd->dev_id, fid, hdcdrv_sevice_str(cmd->service_type), cmd->pid);

    return hdcdrv_server_free(service, cmd->dev_id, cmd->service_type);
}

STATIC long hdcdrv_accept_wait(const struct hdcdrv_dev *dev, struct hdcdrv_service *service, int service_type,
    u64 check_pid, int *session_fd)
{
    long ret;
    struct hdcdrv_connect_list *connect_t = NULL;

    mutex_lock(&service->mutex);

    if (service->listen_status == HDCDRV_INVALID) {
        mutex_unlock(&service->mutex);
        hdcdrv_err_limit("Service has not been created. (dev_id=%d; service_type=\"%s\")\n",
            dev->dev_id, hdcdrv_sevice_str(service_type));
        return HDCDRV_SERVICE_NO_LISTENING;
    }

    if (service->listen_pid != check_pid) {
        mutex_unlock(&service->mutex);
        hdcdrv_err("Device has no permission for service. (dev_id=%d; pid=%llu; service_type=\"%s\"; pid=%llu)\n",
            dev->dev_id, hdcdrv_get_pid(), hdcdrv_sevice_str(service_type), service->listen_pid);
        return HDCDRV_NO_PERMISSION;
    }

    while (service->conn_list_head == NULL) {
        mutex_unlock(&service->mutex);

        ret = (long)wait_event_interruptible(service->wq_conn_avail,
            ((service->conn_list_head != NULL) || (dev->valid != HDCDRV_VALID) ||
            (service->listen_status == HDCDRV_INVALID)));
        if (ret != 0) {
            hdcdrv_warn("Accept wait interruptible exit. (dev_id=%d; service_type=\"%s\"; ret=%d)\n",
                dev->dev_id, hdcdrv_sevice_str(service_type), (int)ret);
            return ret;
        }

        if ((dev->valid != HDCDRV_VALID) || (service->listen_status == HDCDRV_INVALID)) {
            hdcdrv_info("Accept wait quit. (dev_id=%d; service_type=\"%s\"; dev_status=%d; listen_status=%d)\n",
                dev->dev_id, hdcdrv_sevice_str(service_type), dev->valid, service->listen_status);
            return HDCDRV_DEVICE_RESET;
        }

        mutex_lock(&service->mutex);
    }

    connect_t = service->conn_list_head;
    service->conn_list_head = service->conn_list_head->next;

    mutex_unlock(&service->mutex);

    *session_fd = connect_t->session_fd;
    kfree(connect_t);
    connect_t = NULL;

    return HDCDRV_OK;
}

static inline u32 hdccom_get_fid_owner(u32 fid, u32 container_id)
{
    if ((fid > HDCDRV_DEFAULT_PM_FID) && (container_id == HDCDRV_PHY_HOST_ID)) {
        return HDCDRV_SESSION_OWNER_VM;
    }

    if ((fid > HDCDRV_DEFAULT_PM_FID) && (container_id < HDCDRV_DOCKER_MAX_NUM)) {
        return HDCDRV_SESSION_OWNER_CT;
    }

    return HDCDRV_SESSION_OWNER_PM;
}

STATIC long hdcdrv_accept(struct hdcdrv_cmd_accept *cmd, u32 fid)
{
    struct hdcdrv_service *service = NULL;
    struct hdcdrv_session *session = NULL;
    struct hdcdrv_dev *dev = NULL;
    struct hdcdrv_ctrl_msg msg;
    u32 container_id;
    int session_fd;
    u32 create_fid;
    u32 len = 0;
    long ret;
#ifdef CFG_FEATURE_HDC_REG_MEM
    unsigned long flags;
    if (hdc_ctrl->segment == HDCDRV_INVALID_PACKET_SEGMENT) {
        hdcdrv_warn("hdcdrv_connect pcie not ready (dev_id=%d; fid=%u; service_type=\"%s\"; ret=%ld)\n",
            cmd->dev_id, fid, hdcdrv_sevice_str(cmd->service_type), ret);
        return HDCDRV_DEVICE_NOT_READY;
    }
#endif

    ret = hdcdrv_dev_para_check(cmd->dev_id, cmd->service_type);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    container_id = hdcdrv_get_container_id();
    if (container_id >= HDCDRV_DOCKER_MAX_NUM) {
        hdcdrv_err("Parameter container_id illegal. (dev_id=%d; service_type=\"%s\"; container_id=%d)\n",
            cmd->dev_id, hdcdrv_sevice_str(cmd->service_type), container_id);
        return HDCDRV_PARA_ERR;
    }

    dev = &hdc_ctrl->devices[cmd->dev_id];

    create_fid = fid;
#ifdef CFG_FEATURE_VFIO_DEVICE
    if (cmd->service_type == HDCDRV_SERVICE_TYPE_TDT) {
        create_fid = hdcdrv_get_fid(hdcdrv_get_pid());
    }
#endif

    service = hdcdrv_search_service((u32)cmd->dev_id, create_fid, cmd->service_type, cmd->pid);
RETRY_NEXT:
    ret = hdcdrv_accept_wait(dev, service, cmd->service_type, cmd->pid, &session_fd);
    if (ret != HDCDRV_OK) {
        hdcdrv_link_ctrl_msg_stats_add((u32)cmd->dev_id, cmd->service_type, HDCDRV_CTRL_MSG_TYPE_CONNECT,
            HDCDRV_LINK_CTRL_MSG_WAIT_TIMEOUT, (int)ret);
        return ret;
    }
    hdcdrv_link_ctrl_msg_stats_add((u32)cmd->dev_id, cmd->service_type, HDCDRV_CTRL_MSG_TYPE_CONNECT,
        HDCDRV_LINK_CTRL_MSG_WAIT_SUCC, HDCDRV_OK);

    session = &hdc_ctrl->sessions[session_fd];
    session->create_pid = cmd->pid;
    session->container_id = container_id;
    session->local_fid = fid;
    session->owner = hdccom_get_fid_owner(session->local_fid, session->container_id);

#ifdef CFG_FEATURE_HDC_REG_MEM
    if (session->mem_release_fifo.buf != NULL) {
        spin_lock_irqsave(&session->mem_release_lock, flags);
        kfifo_reset(&session->mem_release_fifo);
        spin_unlock_irqrestore(&session->mem_release_lock, flags);
        hdcdrv_info("Clear the mem release fifo when session %d accept.\n", session_fd);
    }
#endif

#ifndef HDC_ENV_DEVICE
    session->run_env = hdcdrv_get_session_run_env((u32)cmd->dev_id, fid);
#endif

    msg.type = HDCDRV_CTRL_MSG_TYPE_CONNECT_REPLY;
    msg.connect_msg_reply.client_session = session->remote_session_fd;
    msg.connect_msg_reply.server_session = session->local_session_fd;
    msg.connect_msg_reply.run_env = session->run_env;
    msg.connect_msg_reply.unique_val = session->unique_val;
    msg.connect_msg_reply.fast_chan_id = session->fast_chan_id;
    msg.connect_msg_reply.normal_chan_id = session->chan_id;
    msg.connect_msg_reply.fid = fid;
    msg.connect_msg_reply.local_pid = cmd->pid;
    msg.error_code = HDCDRV_OK;

    ret = hdcdrv_non_trans_ctrl_msg_send((u32)cmd->dev_id, (void *)&msg, sizeof(msg), sizeof(msg), &len);
    if ((ret != HDCDRV_OK) || (len != sizeof(msg)) || (msg.error_code != HDCDRV_OK)) {
        hdcdrv_mod_msg_chan_session_cnt(session->dev_id, session->chan_id, session->fast_chan_id, -1);
        hdcdrv_session_post_free((u32)session->dev_id, fid, cmd->service_type);
        hdcdrv_set_session_status(session, HDCDRV_SESSION_STATUS_IDLE);
        session->create_pid = HDCDRV_INVALID_PID;
        hdcdrv_err("Reply message send failed. (dev=%d; service=\"%s\"; ret=%ld; len=%d; error_code=%d)\n",
            cmd->dev_id, hdcdrv_sevice_str(cmd->service_type), ret, len, msg.error_code);

        if (((ret == 0)) && (len == sizeof(msg)) && (msg.error_code != HDCDRV_OK) &&
            (service->conn_list_head != NULL)) {
            hdcdrv_link_ctrl_msg_stats_add((u32)cmd->dev_id, cmd->service_type, HDCDRV_CTRL_MSG_TYPE_CONNECT_REPLY,
                HDCDRV_LINK_CTRL_MSG_SEND_FAIL, msg.error_code);
            goto RETRY_NEXT;
        }

        hdcdrv_link_ctrl_msg_stats_add((u32)cmd->dev_id, cmd->service_type, HDCDRV_CTRL_MSG_TYPE_CONNECT_REPLY,
            HDCDRV_LINK_CTRL_MSG_SEND_FAIL, (int)ret);

        return HDCDRV_SEND_CTRL_MSG_FAIL;
    }

    hdcdrv_link_ctrl_msg_stats_add((u32)cmd->dev_id, cmd->service_type, HDCDRV_CTRL_MSG_TYPE_CONNECT_REPLY,
        HDCDRV_LINK_CTRL_MSG_SEND_SUCC, HDCDRV_OK);

    /* response cmd */
    cmd->session = session->local_session_fd;
    session->owner_pid = cmd->pid;

    mutex_lock(&service->mutex);
    service->service_stat.accept_session_num++;
    mutex_unlock(&service->mutex);

    return HDCDRV_OK;
}

STATIC long hdcdrv_connect_wait_reply(const struct hdcdrv_dev *dev, struct hdcdrv_session *session)
{
    long ret;

    ret = wait_event_interruptible_timeout(session->wq_conn,
        (session->remote_session_fd != HDCDRV_INVALID_VALUE) || (dev->valid != HDCDRV_VALID),
        HDCDRV_CONN_TIMEOUT);
    if (ret <= 0) {
        hdcdrv_err("Connect wait error. (dev=%d; ret=%d; local_session_fd=%u; remote_session_fd=0x%x; pid=%llu)\n",
            dev->dev_id, (int)ret, session->local_session_fd, session->remote_session_fd, session->owner_pid);
        return (ret == 0) ? HDCDRV_CONNECT_TIMEOUT : ret;
    }

    if (dev->valid != HDCDRV_VALID) {
        hdcdrv_err("Device invalid. (device=%d; connect_ret=%d; remote_session=%d)\n", dev->dev_id, (int)ret,
            session->remote_session_fd);
        return HDCDRV_DEVICE_RESET;
    }

    if (session->remote_session_fd == HDCDRV_INVALID_REMOTE_SESSION_ID) {
        hdcdrv_err("Channel is invalid. (dev=%d; local_session_fd=%x)\n", dev->dev_id, session->local_session_fd);
        return HDCDRV_SESSION_CHAN_INVALID;
    }

    if (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_IDLE) {
        hdcdrv_err("Session has already closed. (dev_id=%d; local_session_fd=%d; remote_session_fd=%d)\n",
            session->dev_id, session->local_session_fd, session->remote_session_fd);
        return HDCDRV_SESSION_HAS_CLOSED;
    }

    return HDCDRV_OK;
}

bool hdcdrv_is_vf_device(u32 dev_id, u32 fid)
{
    if (fid != HDCDRV_DEFAULT_PM_FID) {
        return true;
    }

    if (devdrv_get_pfvf_type_by_devid(dev_id) == DEVDRV_SRIOV_TYPE_VF) {
        return true;
    }

    return false;
}

STATIC void hdcdrv_uid_privilege_get(struct hdcdrv_ctrl_msg_connect *msg_connect, u32 dev_id, u32 fid)
{
    if ((hdcdrv_is_vf_device(dev_id, fid)) || (current->cred == NULL)) {
        msg_connect->euid = -1;
        msg_connect->uid =  -1;
    } else {
        msg_connect->euid = (int)current->cred->euid.val;
        msg_connect->uid = (int)current->cred->uid.val;
    }

    msg_connect->root_privilege = (msg_connect->euid == 0) ? 1 : 0;
}

STATIC void hdcdrv_close_remote_session(struct hdcdrv_session *session, int close_state)
{
    struct hdcdrv_ctrl_msg msg;
    u32 len = 0;
    long ret;

    msg.type = HDCDRV_CTRL_MSG_TYPE_CLOSE;
    msg.close_msg.local_session = session->local_session_fd;
    msg.close_msg.remote_session = session->remote_session_fd;
    msg.close_msg.session_close_state = close_state;
    msg.close_msg.unique_val = session->unique_val;
    msg.error_code = HDCDRV_OK;  // clear coverity warning

    ret = hdcdrv_non_trans_ctrl_msg_send((u32)session->dev_id, (void *)&msg, sizeof(msg), sizeof(msg), &len);
    if ((ret != HDCDRV_OK) || (len != sizeof(msg)) || (msg.error_code != HDCDRV_OK)) {
        hdcdrv_link_ctrl_msg_stats_add((u32)session->dev_id, session->service_type, HDCDRV_CTRL_MSG_TYPE_CLOSE,
            HDCDRV_LINK_CTRL_MSG_SEND_FAIL, msg.error_code);
        hdcdrv_warn("Close msg send not success. (dev_id=%d; session=%d; service_type=\"%s\"; ret=%ld; len=%d)\n",
            session->dev_id, session->local_session_fd, hdcdrv_sevice_str(session->service_type), ret, len);
    } else {
        hdcdrv_link_ctrl_msg_stats_add((u32)session->dev_id, session->service_type, HDCDRV_CTRL_MSG_TYPE_CLOSE,
            HDCDRV_LINK_CTRL_MSG_SEND_SUCC, HDCDRV_OK);
    }
}

long hdcdrv_connect(struct hdcdrv_cmd_connect *cmd, u32 fid)
{
    struct hdcdrv_dev *dev = NULL;
    struct hdcdrv_service *service = NULL;
    struct hdcdrv_session *session = NULL;
    struct hdcdrv_ctrl_msg msg;
    u32 container_id;
    u32 len = 0;
    int session_fd;
    long ret;
    u32 fast_chan_id;
    u32 normal_chan_id;
    int run_env;
    u32 connect_fid;
    u64 origin_pid;
#ifdef CFG_FEATURE_HDC_REG_MEM
    unsigned long flags;

    if (hdc_ctrl->segment == HDCDRV_INVALID_PACKET_SEGMENT) {
        hdcdrv_warn("hdcdrv_connect pcie not ready (dev_id=%d; fid=%u; service_type=\"%s\"; ret=%ld)\n",
            cmd->dev_id, fid, hdcdrv_sevice_str(cmd->service_type), ret);
        return HDCDRV_DEVICE_NOT_READY;
    }

#endif

    ret = hdcdrv_dev_para_check(cmd->dev_id, cmd->service_type);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    container_id = hdcdrv_get_container_id();
    if (container_id >= HDCDRV_DOCKER_MAX_NUM) {
        hdcdrv_err("container_id is illegal. (dev_id=%d; service_type=\"%s\"; container_id=%d)\n",
            cmd->dev_id, hdcdrv_sevice_str(cmd->service_type), container_id);
        return HDCDRV_PARA_ERR;
    }

    ret = hdcdrv_session_pre_alloc((u32)cmd->dev_id, fid, cmd->service_type);
    if (ret != HDCDRV_OK) {
        hdcdrv_err("Calling session_resouce_check failed. (dev_id=%d; fid=%u; service_type=\"%s\"; ret=%ld)\n",
            cmd->dev_id, fid, hdcdrv_sevice_str(cmd->service_type), ret);
        return ret;
    }

    dev = &hdc_ctrl->devices[cmd->dev_id];
    service = &dev->service[cmd->service_type];

    mutex_lock(&service->mutex);
    service->service_stat.connect_session_num_total++;
    mutex_unlock(&service->mutex);

    connect_fid = fid;

    /* host can get run env, set it here when connect in host */
#ifdef HDC_ENV_DEVICE
    run_env = HDCDRV_SESSION_RUN_ENV_UNKNOW;
    if ((cmd->service_type == HDCDRV_SERVICE_TYPE_FRAMEWORK) ||
        (cmd->service_type == HDCDRV_SERVICE_TYPE_PROFILING) ||
        (cmd->service_type == HDCDRV_SERVICE_TYPE_DUMP) ||
        (cmd->service_type == HDCDRV_SERVICE_TYPE_LOG)) {
        connect_fid = hdcdrv_get_fid(hdcdrv_get_pid());
    }
#else
    run_env = hdcdrv_get_session_run_env((u32)cmd->dev_id, fid);
#endif

    hdcdrv_uid_privilege_get(&msg.connect_msg, (u32)cmd->dev_id, fid);

    session_fd = hdcdrv_alloc_session(cmd->service_type, run_env, msg.connect_msg.root_privilege, cmd->pid);
    if (session_fd < 0) {
        hdcdrv_err_limit("No session resources available. (dev_id=%d; service_type=\"%s\")\n",
            cmd->dev_id, hdcdrv_sevice_str(cmd->service_type));
        hdcdrv_session_post_free((u32)cmd->dev_id, fid, cmd->service_type);
        return HDCDRV_NO_SESSION;
    }

    session = &hdc_ctrl->sessions[session_fd];

#ifdef CFG_FEATURE_HDC_REG_MEM
    if (session->mem_release_fifo.buf != NULL) {
        spin_lock_irqsave(&session->mem_release_lock, flags);
        kfifo_reset(&session->mem_release_fifo);
        spin_unlock_irqrestore(&session->mem_release_lock, flags);
        hdcdrv_info("Clear the mem release fifo when session %d connect.\n", session_fd);
    }
#endif

    session->unique_val = hdcdrv_gen_unique_value();

    hdcdrv_alloc_session_chan(cmd->dev_id, (int)fid, cmd->service_type, &normal_chan_id, &fast_chan_id);

    msg.type = HDCDRV_CTRL_MSG_TYPE_CONNECT;
    msg.error_code = HDCDRV_OK;
    msg.connect_msg.client_session = session_fd;
    msg.connect_msg.service_type = cmd->service_type;
    msg.connect_msg.fast_chan_id = fast_chan_id;
    msg.connect_msg.normal_chan_id = normal_chan_id;
    msg.connect_msg.run_env = run_env;
    msg.connect_msg.fid = connect_fid;
    msg.connect_msg.unique_val = session->unique_val;
    msg.connect_msg.local_pid = cmd->pid;
    msg.connect_msg.peer_pid = (u64)cmd->peer_pid;

    hdcdrv_session_uid(session_fd, msg.connect_msg.euid, msg.connect_msg.uid, msg.connect_msg.root_privilege);
    hdcdrv_session_init(session_fd, HDCDRV_INVALID_VALUE, cmd->dev_id, cmd->service_type, run_env);

    session->container_id = container_id;
    session->local_fid = fid;
    session->owner = hdccom_get_fid_owner(session->local_fid, session->container_id);
    session->chan_id = normal_chan_id;
    session->fast_chan_id = fast_chan_id;

    (void)hdcdrv_session_chan_update(session_fd, cmd->dev_id);
    origin_pid = session->create_pid;
    session->create_pid = cmd->pid;

    ret = hdcdrv_non_trans_ctrl_msg_send((u32)cmd->dev_id, (void *)&msg, (u32)sizeof(msg), (u32)sizeof(msg), &len);
    if ((ret != HDCDRV_OK) || (len != sizeof(msg)) || (msg.error_code != HDCDRV_OK)) {
        session->create_pid = origin_pid;
        hdcdrv_set_session_status(&hdc_ctrl->sessions[session_fd], HDCDRV_SESSION_STATUS_IDLE);
        hdcdrv_session_post_free((u32)cmd->dev_id, fid, cmd->service_type);
        if ((msg.error_code == HDCDRV_REMOTE_SERVICE_NO_LISTENING) || (msg.error_code == HDCDRV_NO_SESSION) ||
            (msg.error_code == HDCDRV_NO_PERMISSION)) {
            hdcdrv_link_ctrl_msg_stats_add((u32)cmd->dev_id, cmd->service_type, HDCDRV_CTRL_MSG_TYPE_CONNECT,
                HDCDRV_LINK_CTRL_MSG_SEND_FAIL, msg.error_code);
            return (long)msg.error_code;
        }

        hdcdrv_link_ctrl_msg_stats_add((u32)cmd->dev_id, cmd->service_type, HDCDRV_CTRL_MSG_TYPE_CONNECT,
            HDCDRV_LINK_CTRL_MSG_SEND_FAIL, (int)ret);
        return HDCDRV_SEND_CTRL_MSG_FAIL;
    }
    hdcdrv_link_ctrl_msg_stats_add((u32)cmd->dev_id, cmd->service_type, HDCDRV_CTRL_MSG_TYPE_CONNECT,
        HDCDRV_LINK_CTRL_MSG_SEND_SUCC, HDCDRV_OK);

    /* conn reply msg maybe come first */
    ret = hdcdrv_connect_wait_reply(dev, session);
    if (ret != HDCDRV_OK) {
        session->create_pid = origin_pid;
        hdcdrv_link_ctrl_msg_stats_add(dev->dev_id, session->service_type, HDCDRV_CTRL_MSG_TYPE_CONNECT_REPLY,
            HDCDRV_LINK_CTRL_MSG_WAIT_TIMEOUT, (int)ret);
        if ((session->remote_session_fd != HDCDRV_INVALID_VALUE) &&
            (session->remote_session_fd != HDCDRV_INVALID_REMOTE_SESSION_ID)) {
            hdcdrv_close_remote_session(session, HDCDRV_CLOSE_TYPE_NOT_SET_OWNER);
        }
        hdcdrv_set_session_status(session, HDCDRV_SESSION_STATUS_IDLE);
        hdcdrv_session_post_free((u32)cmd->dev_id, fid, cmd->service_type);
        hdcdrv_err("Calling hdcdrv_connect_wait_reply failed. (device=%d; service=\"%s\";"
            " ret=%d; local_session_fd=%x; remote_session_fd=%x; pid=%llu)\n",
            cmd->dev_id, hdcdrv_sevice_str(cmd->service_type), (int)ret,
            session->local_session_fd, session->remote_session_fd, session->owner_pid);
        return ret;
    }

    hdcdrv_link_ctrl_msg_stats_add(dev->dev_id, session->service_type, HDCDRV_CTRL_MSG_TYPE_CONNECT_REPLY,
        HDCDRV_LINK_CTRL_MSG_WAIT_SUCC, HDCDRV_OK);
    rmb();
    hdcdrv_mod_msg_chan_session_cnt(session->dev_id, session->chan_id, session->fast_chan_id, 1);

    session->owner_pid = cmd->pid;
    mutex_lock(&service->mutex);
    service->service_stat.connect_session_num++;
    mutex_unlock(&service->mutex);

    /* response cmd */
    cmd->session = session_fd;

    hdcdrv_delay_work_set(session, &session->close_unknow_session, HDCDRV_DELAY_UNKNOWN_SESSION_BIT,
        HDCDRV_SESSION_RECLAIM_TIMEOUT);

    return HDCDRV_OK;
}

STATIC void hdcdrv_session_free(struct hdcdrv_session *session)
{
    struct hdcdrv_service *service = NULL;

    session->container_id = HDCDRV_DOCKER_MAX_NUM;
    session->unique_val = 0;
    session->owner_pid = HDCDRV_INVALID_PID;
    session->pid_flag = 0;

    hdcdrv_delay_work_cancel(session, &session->remote_close, HDCDRV_DELAY_REMOTE_CLOSE_BIT);
    hdcdrv_delay_work_cancel(session, &session->close_unknow_session, HDCDRV_DELAY_UNKNOWN_SESSION_BIT);
#ifdef CFG_FEATURE_VFIO
    hdcdrv_session_work_free(&session->session_work);
#endif
    hdcdrv_mod_msg_chan_session_cnt(session->dev_id, session->chan_id, session->fast_chan_id, -1);
    hdcdrv_unbind_session_ctx(session);
    hdcdrv_inner_checker_clear(session);
    hdcdrv_clear_session_rx_buf(session->local_session_fd);
    hdcdrv_set_session_status(session, HDCDRV_SESSION_STATUS_IDLE);
#ifdef CFG_FEATURE_HDC_REG_MEM
    hdcdrv_destroy_session_del_mem_release_event(session);
#endif
    hdcdrv_session_post_free((u32)session->dev_id, session->local_fid, session->service_type);

    wake_up_interruptible(&session->wq_rx);
    wake_up_interruptible(&session->wq_conn);

    if (session->msg_chan != NULL) {
        wake_up_interruptible(&session->msg_chan->send_wait);
    }

    if (session->fast_msg_chan != NULL) {
        wake_up_interruptible(&session->fast_msg_chan->send_wait);
    }

    hdcdrv_epoll_wake_up(session->epfd);

    service = session->service;
    if (service != NULL) {
        mutex_lock(&service->mutex);
        service->service_stat.close_session_num++;
        mutex_unlock(&service->mutex);
    }
}

static void hdcdrv_invaid_session_creater(struct hdcdrv_session *session, int close_state)
{
    if ((close_state == HDCDRV_CLOSE_TYPE_USER) || (close_state == HDCDRV_CLOSE_TYPE_KERNEL) ||
        (close_state == HDCDRV_CLOSE_TYPE_RELEASE)) {
        session->create_pid = HDCDRV_INVALID_PID;
    }
}

/* this functions are called in remote work and upper layers */
long hdcdrv_close(const struct hdcdrv_cmd_close *cmd, int close_state)
{
    struct hdcdrv_session *session = NULL;
    int last_status = 0;
    long ret;

    if ((cmd->session >= HDCDRV_REAL_MAX_SESSION) || (cmd->session < 0)) {
        hdcdrv_err_limit("Input pararmeter is error. (session=%d)\n", cmd->session);
        return HDCDRV_PARA_ERR;
    }
    session = &hdc_ctrl->sessions[cmd->session];

    ret = hdcdrv_session_state_to_closing(cmd->session, cmd->pid, &last_status, close_state);
    if (ret != HDCDRV_OK) {
        if (ret == HDCDRV_SESSION_HAS_CLOSED) {
            hdcdrv_invaid_session_creater(session, close_state);
            return HDCDRV_OK;
        } else {
            return ret;
        }
    }

    session->local_close_state = close_state;

    /* In case of connection notification peer */
    if (last_status == HDCDRV_SESSION_STATUS_CONN) {
        hdcdrv_close_remote_session(session, close_state);
    }

    hdcdrv_session_free(session);
    hdcdrv_invaid_session_creater(session, close_state);

    return HDCDRV_OK;
}

long hdcdrv_send(struct hdcdrv_cmd_send *cmd, int mode)
{
    struct hdcdrv_session *session = NULL;
    struct hdcdrv_buf_desc buf_d = {0};
    u64 timeout;
    long ret;
    int retry_cnt = 0;

    if (cmd->src_buf == NULL) {
        hdcdrv_err("Input pararmeter is error.\n");
        return HDCDRV_PARA_ERR;
    }

AGAIN:
    ret = hdcdrv_session_valid_check(cmd->session, cmd->dev_id, cmd->pid);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    if (unlikely((cmd->len > hdcdrv_mem_block_capacity()) || (cmd->len <= 0))) {
        hdcdrv_err("Send length is too bigger. (session=%d; send_len=%d; max_segment=%d)\n", cmd->session,
            cmd->len, hdcdrv_mem_block_capacity());
        return HDCDRV_TX_LEN_ERR;
    }

    session = &hdc_ctrl->sessions[cmd->session];

    if (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_REMOTE_CLOSED) {
        hdcdrv_warn_limit("Session status remote close. (dev=%d; session_fd=%d; service_type=\"%s\"; "
            "local_close_state=\"%s\"; remote_close_state=\"%s\"; local_session_fd=%d; remote_session_fd=%d)\n",
            session->dev_id, cmd->session, hdcdrv_sevice_str(session->service_type),
            hdcdrv_close_str(session->local_close_state), hdcdrv_close_str(session->remote_close_state),
            session->local_session_fd, session->remote_session_fd);

        return HDCDRV_TX_REMOTE_CLOSE;
    }

    ret = hdcdrv_tx_alloc_mem(session, cmd, &buf_d);
    if (ret != HDCDRV_OK) {
        retry_cnt++;
        if (retry_cnt > HDCDRV_SEND_ALLOC_MEM_RETRY_TIME) {
            hdcdrv_err_limit("Send alloc dma memory failed. (dev=%d; driver=\"%s\"; session=%d; service_type=\"%s\")\n",
                session->dev_id, dev_driver_string(session->msg_chan->dev), cmd->session,
                hdcdrv_sevice_str(session->service_type));
            return HDCDRV_DMA_MEM_ALLOC_FAIL;
        }

        msleep(5);
        goto AGAIN;
    }

    ret = hdcdrv_copy_from_user(session, buf_d.buf, cmd, mode);
    if (ret != HDCDRV_OK) {
        hdcdrv_free_mem(session, buf_d.buf, HDCDRV_MEMPOOL_FREE_IN_VM, &buf_d);
        hdcdrv_err("Calling copy_from_user failed. (dev=%d; driver=\"%s\"; session=%d; service_type=\"%s\"; ret=%ld)\n",
            session->dev_id, dev_driver_string(session->msg_chan->dev), cmd->session,
            hdcdrv_sevice_str(session->service_type), ret);
        return ret;
    }

    buf_d.len = cmd->len;
    buf_d.local_session = session->local_session_fd;
    buf_d.remote_session = session->remote_session_fd;
    buf_d.skip_flag = HDCDRV_INVALID;
    timeout = hdcdrv_set_send_timeout(session, cmd->timeout);

#ifdef CFG_FEATURE_MIRROR
    dma_sync_single_for_device(session->msg_chan->dev, buf_d.addr, (u32)buf_d.len, DMA_TO_DEVICE);
#endif

    ret = hdcdrv_msg_chan_send(session->msg_chan, &buf_d, cmd->wait_flag, timeout);
    if (ret != HDCDRV_OK) {
        hdcdrv_free_mem(session, buf_d.buf, HDCDRV_MEMPOOL_FREE_IN_VM, &buf_d);
    }

    return ret;
}

long hdcdrv_recv_peek(struct hdcdrv_cmd_recv_peek *cmd)
{
    struct hdcdrv_session *session = NULL;
    long ret;
    u64 timeout;
    int tmp_head;

    ret = hdcdrv_session_valid_check(cmd->session, cmd->dev_id, cmd->pid);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    session = &hdc_ctrl->sessions[cmd->session];
    spin_lock_bh(&session->lock);

    timeout = hdcdrv_set_send_timeout(session, cmd->timeout);

    while (((hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_CONN) ||
        (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_REMOTE_CLOSED)) &&
        (session->normal_rx.head == session->normal_rx.tail)) {
        spin_unlock_bh(&session->lock);

        if (cmd->wait_flag == HDCDRV_NOWAIT) {
            return HDCDRV_NO_BLOCK;
        } else if (cmd->wait_flag == HDCDRV_WAIT_ALWAYS) {
            /*lint -e666*/
            ret = (long)wait_event_interruptible(session->wq_rx,
                (session->normal_rx.head != session->normal_rx.tail) ||
                (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_IDLE));
            /*lint +e666*/
        } else {
            /*lint -e666*/
            ret = wait_event_interruptible_timeout(session->wq_rx,
                (session->normal_rx.head != session->normal_rx.tail) ||
                (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_IDLE),
                (long)timeout);
            /*lint +e666*/
            if (ret == 0) {
                return HDCDRV_RX_TIMEOUT;
            } else if (ret > 0) {
                ret = 0;
            }
        }
        if (ret != 0) {
            hdcdrv_warn("Get session.(dev=%d; session=%d; service_type=\"%s\"; recv_peek_wait_ret=%d;"
                "head=%d; tail=%d; local_close_state=\"%s\"; remote_close_state=\"%s\";"
                "local_session_fd=%d; remote_session_fd=%d)\n", session->dev_id, cmd->session,
                hdcdrv_sevice_str(session->service_type), (int)ret, session->normal_rx.head,
                session->normal_rx.tail, hdcdrv_close_str(session->local_close_state),
                hdcdrv_close_str(session->remote_close_state), session->local_session_fd,
                session->remote_session_fd);
            return ret;
        }

        spin_lock_bh(&session->lock);
    }

    /* session is closed */
    if (session->normal_rx.head == session->normal_rx.tail) {
        cmd->len = 0;
        cmd->count = 0;
        spin_unlock_bh(&session->lock);
        hdcdrv_info("Session local or remote close. (dev=%d; session=%d; local_close_state=\"%s\"; "
            "remote_close_state=\"%s\"; local_session_fd=%d; remote_session_fd=%d)\n",
            session->dev_id, cmd->session, hdcdrv_close_str(session->local_close_state),
            hdcdrv_close_str(session->remote_close_state), session->local_session_fd, session->remote_session_fd);
        return HDCDRV_OK;
    }

    if (cmd->group_flag == 0) {  // 0 is normal recv, return one rx len to user
        cmd->len = session->normal_rx.rx_list[session->normal_rx.head].len;
        cmd->count = 1;
    } else {
        tmp_head = session->normal_rx.head;
        cmd->len = 0;
        cmd->count = 0;
        while (tmp_head != session->normal_rx.tail) {
            cmd->len += session->normal_rx.rx_list[tmp_head].len;
            cmd->count++;
            tmp_head = (tmp_head + 1) % HDCDRV_SESSION_RX_LIST_MAX_PKT;
        }
    }

    spin_unlock_bh(&session->lock);
    return HDCDRV_OK;
}

long hdcdrv_recv(struct hdcdrv_cmd_recv *cmd, int mode)
{
    struct hdcdrv_session *session = NULL;
    struct hdcdrv_dev *hdc_dev = NULL;
    struct hdcdrv_buf_desc *buf_desc = NULL;
    struct hdcdrv_msg_chan *msg_chan = NULL;
#ifdef CFG_FEATURE_VFIO
    struct sg_table *dma_sgt[HDCDRV_SESSION_RX_LIST_MAX_PKT] = {0};
    u32 dev_id[HDCDRV_SESSION_RX_LIST_MAX_PKT] = {0};
    u32 fid[HDCDRV_SESSION_RX_LIST_MAX_PKT] = {0};
#endif
#ifdef CFG_FEATURE_MIRROR
    int mem_id_list[HDCDRV_SESSION_RX_LIST_MAX_PKT] = {0};
#endif
    long ret;
    int len, count = 0, rx_count = 0, rx_index = 0, real_len = 0;

    if ((cmd->dst_buf == NULL) || (cmd->buf_count >= HDCDRV_SESSION_RX_LIST_MAX_PKT) || (cmd->buf_count <= 0)) {
        hdcdrv_err("Input pararmeter is error.(buf_count=%d)\n", cmd->buf_count);
        return HDCDRV_PARA_ERR;
    }
    if (mode != HDCDRV_MODE_KERNEL) {
        ret = hdcdrv_session_valid_check(cmd->session, cmd->dev_id, cmd->pid);
        if (ret != 0) {
            return ret;
        }
    }
    session = &hdc_ctrl->sessions[cmd->session];

    hdc_dev = &hdc_ctrl->devices[session->dev_id];
    msg_chan = hdc_dev->msg_chan[session->chan_id];

    mutex_lock(&msg_chan->mutex);
    msg_chan->dbg_stat.hdcdrv_recv_data_times++;
    session->dbg_stat.hdcdrv_recv_data_times++;
    mutex_unlock(&msg_chan->mutex);

    spin_lock_bh(&session->lock);
    session->stat.rx_total++;
    /* session is closed or no data here */
    /* Some services use the length as the termination receiving condition, so no log is printed */
    if (session->normal_rx.head == session->normal_rx.tail) {
        cmd->out_len = 0;
        spin_unlock_bh(&session->lock);
        return HDCDRV_OK;
    }

    /* we should get buf addr from the head postion of rx_list firstly.
       when move head, unlock, in the old head postion of rx_list will be use in other thread */
    rx_index = session->normal_rx.head;

    while ((rx_index != session->normal_rx.tail) && (rx_count < cmd->buf_count)) {
        buf_desc = &session->normal_rx.rx_list[rx_index];
        len = buf_desc->len;
        real_len += len;
        if (real_len > cmd->len) {                // The actual required length exceeds the reserved length
            spin_unlock_bh(&session->lock);
            hdcdrv_err("cmd_len is invalid. (dev=%d; driver=\"%s\"; session=%d; service type=\"%s\"; "
                "recv_buf=%d; len=%d; buf_count=%d; now_buf=%d)\n", session->dev_id, dev_driver_string(hdc_dev->dev),
                cmd->session, hdcdrv_sevice_str(session->service_type), cmd->len, real_len, cmd->buf_count, rx_count);
            return HDCDRV_RX_BUF_SMALL;
        }
        cmd->buf_list[rx_count] = buf_desc->buf;
        cmd->buf_len[rx_count] = (unsigned int)buf_desc->len;
        rx_count++;
        rx_index = (session->normal_rx.head + rx_count) % HDCDRV_SESSION_RX_LIST_MAX_PKT;
    }

    if (rx_count != cmd->buf_count) {
        spin_unlock_bh(&session->lock);
        hdcdrv_err("buf count is invalid.  (dev=%d; driver=\"%s\"; session=%d; service type=\"%s\"; "
            "cmd->buf_count=%d; real buf_count=%d).\n", session->dev_id, dev_driver_string(hdc_dev->dev),
            cmd->session, hdcdrv_sevice_str(session->service_type), cmd->buf_count, rx_count);
        return HDCDRV_RX_BUF_SMALL;
    }

    count = 0;
    while (count < rx_count) {
        rx_index = (session->normal_rx.head + count) % HDCDRV_SESSION_RX_LIST_MAX_PKT;
        buf_desc = &session->normal_rx.rx_list[rx_index];
#ifdef CFG_FEATURE_VFIO
        dma_sgt[count] = buf_desc->dma_sgt;
        dev_id[count] = buf_desc->dev_id;
        fid[count] = buf_desc->fid;
#endif
#ifdef CFG_FEATURE_MIRROR
        mem_id_list[count] = buf_desc->mem_id;
#endif
        hdcdrv_desc_clear(buf_desc);
        count++;
    }

    session->normal_rx.head = (session->normal_rx.head + rx_count) % HDCDRV_SESSION_RX_LIST_MAX_PKT;
    if (msg_chan->rx_trigger_flag == HDCDRV_VALID) {
        msg_chan->rx_trigger_flag = HDCDRV_INVALID;
        tasklet_schedule(&msg_chan->rx_task);
    }

    spin_unlock_bh(&session->lock);

#ifdef CFG_FEATURE_VFIO
        if (session->owner == HDCDRV_SESSION_OWNER_VM) {
            for (count = 0; count < rx_count; count++) {
                hdcdrv_dma_unmap_guest_page(dev_id[count], fid[count], dma_sgt[count]);
            }
        }
#endif

#ifdef CFG_FEATURE_MIRROR
    ret = hdcdrv_copy_to_user(session, cmd, real_len, mode, mem_id_list);
#else
    ret = hdcdrv_copy_to_user(session, cmd, real_len, mode, NULL);
#endif
    return ret;
}

STATIC int hdcdrv_calc_page_start_idx(u64 send_va, int len, struct hdcdrv_fast_mem *f_mem,
    u32 *page_start_idx, u32 *page_nums)
{
#ifdef CFG_FEATURE_HDC_REG_MEM
    u32 page_size = f_mem->align_size;

    if (page_size == 0) {
        *page_start_idx = 0;
        *page_nums = (u32)f_mem->phy_addr_num;
        f_mem->send_inner_page_offset = 0;
        f_mem->page_start_idx = 0;
        return HDCDRV_OK;
    }
    if (send_va < f_mem->user_va) {
        hdcdrv_err("hdcdrv_calc_page_start_idx, (send_va=0x%llx; va=0x%llx; page_size=%u)\n",
                   send_va, f_mem->user_va, page_size);
        return HDCDRV_ERR;
    }

    *page_start_idx = (u32)(send_va - (f_mem->user_va - f_mem->register_inner_page_offset)) / page_size;
    *page_nums = (u32)((len + (send_va % page_size) + page_size - 1) / page_size);
    f_mem->send_inner_page_offset = (u32)(send_va % page_size);
    f_mem->page_start_idx = *page_start_idx;
    if ((*page_start_idx + *page_nums) > (u32)f_mem->phy_addr_num) {
        hdcdrv_err("hdcdrv_calc_page_start_idx, (send_va=0x%llx; va=0x%llx len=%d)\n", send_va,
            f_mem->user_va, len);
        return HDCDRV_ERR;
    }
#else
    (void)send_va;
    (void)len;
    *page_start_idx = 0;
    *page_nums = (u32)f_mem->phy_addr_num;
#endif
    return HDCDRV_OK;
}

STATIC int hdcdrv_sync_used_fast_mem(struct hdcdrv_fast_mem *f_mem, struct device *dev,
    int type, int len, u64 user_va)
{
    u32 i, page_start, page_end, page_nums;
    u32 dma_len, total_len;
    dma_addr_t addr;
    int ret;

    ret = hdcdrv_calc_page_start_idx(user_va, len, f_mem, &page_start, &page_nums);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    total_len = 0;
    page_end = page_start + page_nums;
    for (i = page_start; i < page_end; i++) {
        addr = f_mem->mem[i].addr;
        dma_len = f_mem->mem[i].len;
#ifdef CFG_FEATURE_HDC_REG_MEM
        if (i == page_start) {
            if (f_mem->page_start_idx == 0) {
                addr = f_mem->mem[i].addr + (f_mem->send_inner_page_offset - f_mem->register_inner_page_offset);
            } else {
                addr = f_mem->mem[i].addr + f_mem->send_inner_page_offset;
            }

            dma_len = min((u32)len, f_mem->align_size - f_mem->send_inner_page_offset);
        } else if (i == page_end - 1) {
            dma_len = (u32)(len - total_len);
        }
#endif
        /* dma_map_page or dma_map_single address, cache consistency is not
        guaranteed(arm), need to cooperate with dma_sync_single */
        if ((type == HDCDRV_FAST_MEM_TYPE_TX_DATA) || (type == HDCDRV_FAST_MEM_TYPE_TX_CTRL)) {
            dma_sync_single_for_device(dev, addr, dma_len, DMA_TO_DEVICE);
        } else {
            dma_sync_single_for_cpu(dev, addr, dma_len, DMA_FROM_DEVICE);
        }

        total_len += dma_len;
    }

    return HDCDRV_OK;
}

STATIC void hdcdrv_sync_fast_mem(const struct hdcdrv_session *session,
    int type, u64 user_va, int len, u64 hash_va)
{
    struct hdcdrv_fast_mem *f_mem = NULL;
    long ret;
#ifdef CFG_FEATURE_HDC_REG_MEM
    if ((len == 0) || (user_va == 0)) {
        return;
    }
#endif

    f_mem = hdcdrv_get_fast_mem_timeout(session->dev_id, type, len, hash_va, user_va);
    if (f_mem == NULL) {
        return;
    }

    ret = hdcdrv_sync_used_fast_mem(f_mem, session->fast_msg_chan->dev, type, len, user_va);
    if (ret != HDCDRV_OK) {
        return;
    }

    hdcdrv_node_status_idle_by_mem(f_mem);
}

STATIC long hdcdrv_fast_send_check(const struct hdcdrv_cmd_fast_send *cmd)
{
    long ret;

    ret = hdcdrv_session_valid_check(cmd->session, cmd->dev_id, cmd->pid);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    if (((cmd->src_data_addr == 0) && (cmd->src_ctrl_addr == 0)) ||
        ((cmd->data_len == 0) && (cmd->ctrl_len == 0))) {
        hdcdrv_err("Fast send addr or data len not correct.\n");
        return HDCDRV_PARA_ERR;
    }
#ifdef CFG_FEATURE_HDC_REG_MEM
    if ((cmd->src_data_addr != round_down(cmd->src_data_addr, HDCDRV_MEM_CACHE_LINE)) ||
        (cmd->src_ctrl_addr != round_down(cmd->src_ctrl_addr, HDCDRV_MEM_CACHE_LINE))) {
        hdcdrv_err("Fast send addr not align cacheline, src_data_addr=%llx src_ctrl_addr=%llx.\n",
            cmd->src_data_addr, cmd->src_ctrl_addr);
        return HDCDRV_PARA_ERR;
    }

    if ((cmd->dst_data_addr != round_down(cmd->dst_data_addr, HDCDRV_MEM_CACHE_LINE)) ||
        (cmd->dst_ctrl_addr != round_down(cmd->dst_ctrl_addr, HDCDRV_MEM_CACHE_LINE))) {
        hdcdrv_err("Fast send addr not align cacheline, dst_data_addr=%llx dst_ctrl_addr=%llx.\n",
            cmd->dst_data_addr, cmd->dst_ctrl_addr);
        return HDCDRV_PARA_ERR;
    }
#endif
    if ((cmd->data_len < 0) || (cmd->ctrl_len < 0)) {
        hdcdrv_err("Fast send length invalid.\n");
        return HDCDRV_PARA_ERR;
    }

    return HDCDRV_OK;
}

STATIC long hdcdrv_fast_send(const struct hdcdrv_cmd_fast_send *cmd)
{
    struct hdcdrv_session *session = NULL;
    struct hdcdrv_buf_desc buf_d = {0};
    u64 timeout;
    u32 fid;
    long ret;

    ret = hdcdrv_fast_send_check(cmd);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    session = &hdc_ctrl->sessions[cmd->session];

    if (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_REMOTE_CLOSED) {
        hdcdrv_warn_limit("Session remote closed. (dev=%d; session_fd=%d; service_type=\"%s\"; "
            "local_close_state=\"%s\"; remote_close_state=\"%s\"; local_session_fd=%d; remote_session_fd=%d)\n",
            session->dev_id, cmd->session, hdcdrv_sevice_str(session->service_type),
            hdcdrv_close_str(session->local_close_state), hdcdrv_close_str(session->remote_close_state),
            session->local_session_fd, session->remote_session_fd);
        return HDCDRV_TX_REMOTE_CLOSE;
    }

#ifdef CFG_FEATURE_HDC_REG_MEM
    if (kfifo_is_full(&session->mem_release_fifo) != 0) {
        hdcdrv_warn_limit("Session Mem trans completion info is full to be obtained. (dev=%d; session_fd=%d; "
            "local_session_fd=%d; remote_session_fd=%d)\n", session->dev_id, cmd->session,
            session->local_session_fd, session->remote_session_fd);
    }
#endif

    fid = (session->owner == HDCDRV_SESSION_OWNER_VM) ? session->local_fid : 0;

    buf_d.local_session = session->local_session_fd;
    buf_d.remote_session = session->remote_session_fd;

    /* data pair */
    buf_d.addr = hdcdrv_get_hash(cmd->src_data_addr, cmd->pid, fid);
    buf_d.src_data_addr_va = cmd->src_data_addr;
    buf_d.len = cmd->data_len;

    buf_d.dst_data_addr = cmd->dst_data_addr;

    /* strl msg pair */
    buf_d.src_ctrl_addr = hdcdrv_get_hash(cmd->src_ctrl_addr, cmd->pid, fid);
    buf_d.src_ctrl_addr_va = cmd->src_ctrl_addr;
    buf_d.ctrl_len = cmd->ctrl_len;

    buf_d.dst_ctrl_addr = cmd->dst_ctrl_addr;
    buf_d.skip_flag = HDCDRV_INVALID;
    buf_d.buf = NULL;
    buf_d.status = 0;
    buf_d.pid = (u32)cmd->pid;
    buf_d.fid = fid;
    timeout = (u64)hdcdrv_set_fast_send_timeout(session, cmd->timeout);

    hdcdrv_sync_fast_mem(session, HDCDRV_FAST_MEM_TYPE_TX_DATA,
        cmd->src_data_addr, cmd->data_len, buf_d.addr);

    hdcdrv_sync_fast_mem(session, HDCDRV_FAST_MEM_TYPE_TX_CTRL,
        cmd->src_ctrl_addr, cmd->ctrl_len, buf_d.src_ctrl_addr);

    return hdcdrv_msg_chan_send(session->fast_msg_chan, &buf_d, cmd->wait_flag, timeout);
}

long hdcdrv_fast_recv(struct hdcdrv_cmd_fast_recv *cmd)
{
    struct hdcdrv_session *session = NULL;
    struct hdcdrv_msg_chan *msg_chan = NULL;
    struct hdcdrv_session_fast_rx *fast_rx = NULL;
    struct hdcdrv_fast_rx *rx_buf = NULL;
    u64 timeout;
    u32 fid;
    long ret;

    ret = hdcdrv_session_valid_check(cmd->session, cmd->dev_id, cmd->pid);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    session = &hdc_ctrl->sessions[cmd->session];
    msg_chan = hdc_ctrl->devices[session->dev_id].msg_chan[session->fast_chan_id];

    mutex_lock(&msg_chan->mutex);
    msg_chan->dbg_stat.hdcdrv_recv_data_times++;
    mutex_unlock(&msg_chan->mutex);

    fast_rx = hdcdrv_get_session_fast_rx(cmd->session);

    spin_lock_bh(&session->lock);
    session->stat.rx_total++;
    timeout = hdcdrv_set_send_timeout(session, cmd->timeout);
    while (
        ((hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_CONN) ||
        (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_REMOTE_CLOSED)) &&
        (fast_rx->head == fast_rx->tail)) {
        spin_unlock_bh(&session->lock);
        if (cmd->wait_flag == HDCDRV_NOWAIT) {
            return HDCDRV_NO_BLOCK;
        } else if (cmd->wait_flag == HDCDRV_WAIT_ALWAYS) {
            /*lint -e666*/
            ret = (long)wait_event_interruptible(session->wq_rx,
                (fast_rx->head != fast_rx->tail) ||
                (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_IDLE));
            /*lint +e666*/
        } else {
            /*lint -e666*/
            ret = wait_event_interruptible_timeout(session->wq_rx,
                (fast_rx->head != fast_rx->tail) ||
                (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_IDLE),
                (long)timeout);
            /*lint +e666*/
            if (ret == 0) {
                return HDCDRV_RX_TIMEOUT;
            } else if (ret > 0) {
                ret = 0;
            }
        }

        if (ret != 0) {
            hdcdrv_warn("Get session. (dev=%d; session=%d; service_type=\"%s\"; fast_recv_wait_head=%d; "
                "tail=%d; ret=%d; local_close_state=\"%s\"; remote_close_state=\"%s\"; local_session_fd=%d;"
                "remote_session_fd=%d)\n", session->dev_id, cmd->session,
                hdcdrv_sevice_str(session->service_type), fast_rx->head, fast_rx->tail, (int)ret,
                hdcdrv_close_str(session->local_close_state), hdcdrv_close_str(session->remote_close_state),
                session->local_session_fd, session->remote_session_fd);
            return ret;
        }

        spin_lock_bh(&session->lock);
    }

    /* session is closed */
    if (fast_rx->head == fast_rx->tail) {
        spin_unlock_bh(&session->lock);
        cmd->ctrl_addr = 0;
        cmd->data_addr = 0;
        cmd->data_len = 0;
        cmd->ctrl_len = 0;

        hdcdrv_info("Session local or remote close. (dev=%d; session=%d; local_close_state=\"%s\"; "
            "remote_close_state=\"%s\"; local_session_fd=%d; remote_session_fd=%d)\n", session->dev_id, cmd->session,
            hdcdrv_close_str(session->local_close_state), hdcdrv_close_str(session->remote_close_state),
            session->local_session_fd, session->remote_session_fd);

        return HDCDRV_OK;
    }

    rx_buf = &fast_rx->rx_list[fast_rx->head];

    /* Insert into the tail of the receive list */
    cmd->data_addr = (u64)rx_buf->data_addr;
    cmd->ctrl_addr = (u64)rx_buf->ctrl_addr;
    cmd->data_len = rx_buf->data_len;
    cmd->ctrl_len = rx_buf->ctrl_len;
    fast_rx->head = (fast_rx->head + 1) % HDCDRV_BUF_MAX_CNT;
    if (msg_chan->rx_trigger_flag == HDCDRV_VALID) {
        msg_chan->rx_trigger_flag = HDCDRV_INVALID;
        tasklet_schedule(&msg_chan->rx_task);
    }
    spin_unlock_bh(&session->lock);

    fid = (session->owner == HDCDRV_SESSION_OWNER_VM) ? session->local_fid : 0;

    hdcdrv_sync_fast_mem(session, HDCDRV_FAST_MEM_TYPE_RX_DATA,
        cmd->data_addr, cmd->data_len, hdcdrv_get_hash(cmd->data_addr, session->owner_pid, fid));

    hdcdrv_sync_fast_mem(session, HDCDRV_FAST_MEM_TYPE_RX_CTRL,
        cmd->ctrl_addr, cmd->ctrl_len, hdcdrv_get_hash(cmd->ctrl_addr, session->owner_pid, fid));

    return HDCDRV_OK;
}

STATIC long hdcdrv_set_session_owner(struct hdcdrv_ctx *ctx, struct hdcdrv_cmd_set_session_owner *cmd, u32 fid)
{
    struct hdcdrv_session *session = NULL;
    long ret;

    if (ctx == NULL) {
        hdcdrv_err("Input pararmeter is error. (session_id=%d)\n", cmd->session);
        return HDCDRV_PARA_ERR;
    }

    ret = hdcdrv_session_para_check(cmd->session, cmd->dev_id);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    if (fid == HDCDRV_DEFAULT_PM_FID) {
        cmd->ppid = hdcdrv_rebuild_pid((u32)cmd->dev_id, fid, hdcdrv_get_ppid());
    }

    session = &hdc_ctrl->sessions[cmd->session];
    if ((session->create_pid != cmd->pid) && (session->create_pid != cmd->ppid)) {
        hdcdrv_err("No permission for session. (session=%d; creat_pid=%llu; cmd_pid=%llu; ppid=%llu)\n",
            cmd->session, session->create_pid, cmd->pid, cmd->ppid);
        return HDCDRV_NO_PERMISSION;
    }

    if ((session->owner_pid != session->create_pid) || (session->pid_flag != 0)) {
        hdcdrv_err("Current pid is invalid. (Current_pid=%llu; session_id=%d; owner_pid=%llu)\n",
            cmd->pid, cmd->session, session->owner_pid);
        return HDCDRV_ERR;
    }

    session->pid_flag = 1;
    if ((session->delay_work_flag & BIT(HDCDRV_DELAY_UNKNOWN_SESSION_BIT)) == 0) {
        hdcdrv_warn_limit("delay_work_cancel not success. (status=%d, local_close=%d, remote_close=%d, session=%d,"
            "service=%d).\n", hdcdrv_get_session_status(session), session->local_close_state,
            session->remote_close_state, cmd->session, session->service_type);
    }

    hdcdrv_delay_work_cancel(session, &session->close_unknow_session, HDCDRV_DELAY_UNKNOWN_SESSION_BIT);
    session->owner_pid = cmd->pid;
    hdcdrv_bind_sessoin_ctx(ctx, session);

    if (hdcdrv_get_session_status(session) != HDCDRV_SESSION_STATUS_CONN) {
        hdcdrv_warn_limit("Session not conn when set owner. (status=%d, local_close=%d, remote_close=%d, session=%d,"
            "service=%d).\n", hdcdrv_get_session_status(session), session->local_close_state,
            session->remote_close_state, cmd->session, session->service_type);
    }

    return HDCDRV_OK;
}

STATIC int hdcdrv_attr_get_session_status(struct hdcdrv_cmd_get_session_attr *cmd)
{
    int ret;

    if ((cmd->session >= HDCDRV_REAL_MAX_SESSION) || (cmd->session < 0)) {
        hdcdrv_err_limit("session_fd is illegal. (session_fd=%d)\n", cmd->session);
        return HDCDRV_PARA_ERR;
    }

    ret = hdcdrv_session_valid_check(cmd->session, cmd->dev_id, cmd->pid);
    if (ret != HDCDRV_OK) {
        return HDCDRV_SESSION_HAS_CLOSED;
    }

    /* session is connect */
    return HDCDRV_OK;
}

STATIC long hdcdrv_get_session_dfx(struct hdcdrv_cmd_get_session_attr *cmd)
{
    ssize_t offset = 0;
    int ret;
    u32 msg_len, out_len;
    struct hdcdrv_sysfs_ctrl_msg *msg = NULL;
    struct hdcdrv_session *session = NULL;
    void* buf = NULL;

    session = &hdc_ctrl->sessions[cmd->session];
    buf = hdcdrv_kvmalloc(HDCDRV_LOG_MAX_LEN);
    if (buf == NULL) {
        hdcdrv_err("Calling kzalloc failed.\n");
        return HDCDRV_ERR;
    }

    // get local session dfx
    ret = hdcdrv_sysfs_get_session_inner(buf, HDCDRV_LOG_MAX_LEN, (u32)cmd->session, &offset);
    if (ret != HDCDRV_OK) {
        goto dfx_buf_exit;
    }
    hdcdrv_event("%s\n", (char*)buf);

    // clear buf for recv remote session dfx
    (void)memset_s(buf, HDCDRV_LOG_MAX_LEN, 0, HDCDRV_LOG_MAX_LEN);
    offset = 0;

    msg_len = (u32)sizeof(struct hdcdrv_sysfs_ctrl_msg) + HDCDRV_SYSFS_DATA_MAX_LEN;
    msg = (struct hdcdrv_sysfs_ctrl_msg *)hdcdrv_kvmalloc(msg_len);
    if (msg == NULL) {
        hdcdrv_err("Calling kzalloc failed. (dev_id=%u; server_type=%u)\n", cmd->dev_id, session->service_type);
        ret = HDCDRV_ERR;
        goto dfx_buf_exit;
    }

    ret = snprintf_s(buf + offset, (size_t)(HDCDRV_LOG_MAX_LEN - offset),
        (size_t)(HDCDRV_LOG_MAX_LEN - offset - 1), "Remote session stat:\n");
    if (ret >= 0) {
        offset += ret;
    }

    // get remote session dfx
    msg->head.type = HDCDRV_CTRL_MSG_TYPE_GET_DEV_SESSION_STAT;
    msg->head.error_code = HDCDRV_OK;
    msg->head.para = (u32)session->remote_session_fd;
    msg->head.msg_len = msg_len;
    ret = (int)hdcdrv_ctrl_msg_send((u32)session->dev_id, (void *)msg, msg_len, msg_len, &out_len);
    // buf_len is HDCDRV_LOG_MAX_LEN, which is used as the boundary check length
    if ((ret == 0) && (msg->head.msg_len < HDCDRV_LOG_MAX_LEN - offset) && (msg->head.msg_len > 0)) {
        msg->data[msg->head.msg_len - 1] = '\0';
        ret = snprintf_s(buf + offset, (size_t)(HDCDRV_LOG_MAX_LEN - offset),
            (size_t)(HDCDRV_LOG_MAX_LEN - offset - 1), "%s\n", msg->data);
        if (ret >= 0) {
            offset += ret;
        }
    }
    hdcdrv_event("%s\n", (char*)buf);

    hdcdrv_kvfree(msg);
    msg = NULL;
dfx_buf_exit:
    hdcdrv_kvfree(buf);
    buf = NULL;

    return (long)ret;
}

STATIC long hdcdrv_get_session_attr(struct hdcdrv_cmd_get_session_attr *cmd)
{
    struct hdcdrv_session *session = NULL;
    long ret;

    /* get session status should before hdcdrv_session_valid_check */
    if (cmd->cmd_type == HDCDRV_SESSION_ATTR_STATUS) {
        ret = hdcdrv_attr_get_session_status(cmd);
        if (ret == HDCDRV_PARA_ERR) {
            return ret;
        }
        cmd->output = (int)ret;
        return HDCDRV_OK;
    }

    ret = hdcdrv_session_valid_check(cmd->session, cmd->dev_id, cmd->pid);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    session = &hdc_ctrl->sessions[cmd->session];

    switch (cmd->cmd_type) {
        case HDCDRV_SESSION_ATTR_RUN_ENV:
            cmd->output = session->run_env;
            break;
        case HDCDRV_SESSION_ATTR_VFID:
            cmd->output = (int)session->remote_fid;
            break;
        case HDCDRV_SESSION_ATTR_LOCAL_CREATE_PID:
            cmd->output = hdcdrv_rebuild_raw_pid(session->create_pid);
            break;
        case HDCDRV_SESSION_ATTR_PEER_CREATE_PID:
            cmd->output = hdcdrv_rebuild_raw_pid(session->peer_create_pid);
            break;
        case HDCDRV_SESSION_ATTR_DFX:
            cmd->output = (int)hdcdrv_get_session_dfx(cmd);
            break;
        default:
            hdcdrv_err("Input pararmeter is error.\n");
            ret = HDCDRV_ERR;
            break;
    }

    return ret;
}

STATIC long hdcdrv_set_session_timeout(const struct hdcdrv_cmd_set_session_timeout *cmd)
{
    struct hdcdrv_session *session = NULL;
    long ret;
    u64 send_timeout;
    u64 recv_timeout;
    u64 fast_send_timeout;
    u64 fast_recv_timeout;

    ret = hdcdrv_session_valid_check(cmd->session, cmd->dev_id, cmd->pid);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    session = &hdc_ctrl->sessions[cmd->session];

    send_timeout = msecs_to_jiffies(cmd->timeout.send_timeout);
    recv_timeout = msecs_to_jiffies(cmd->timeout.recv_timeout);
    fast_send_timeout = msecs_to_jiffies(cmd->timeout.fast_send_timeout);
    fast_recv_timeout = msecs_to_jiffies(cmd->timeout.fast_recv_timeout);

    if (cmd->timeout.send_timeout != 0) {
        session->timeout_jiffies.send_timeout = send_timeout;
    }

    if (cmd->timeout.fast_send_timeout != 0) {
        session->timeout_jiffies.fast_send_timeout = fast_send_timeout;
    }

    return HDCDRV_OK;
}

STATIC long hdcdrv_get_uid_state(struct hdcdrv_cmd_get_uid_stat *cmd)
{
    struct hdcdrv_session *session = NULL;
    int ret;

    ret = (int)hdcdrv_session_valid_check(cmd->session, cmd->dev_id, cmd->pid);
    if (ret != HDCDRV_OK) {
        hdcdrv_info("Parameters checked invalid.\n");
        return ret;
    }
    session = &hdc_ctrl->sessions[cmd->session];

    cmd->euid = (unsigned int)session->euid;
    cmd->uid = (unsigned int)session->uid;
    cmd->root_privilege = session->root_privilege;

    return HDCDRV_OK;
}

STATIC long hdcdrv_get_session_info(struct hdcdrv_cmd_get_session_info *cmd)
{
    struct hdcdrv_session *session = NULL;
    long ret;

    ret = hdcdrv_session_valid_check(cmd->session_fd, cmd->dev_id, cmd->pid);
    if (ret != 0) {
        return ret;
    }

    session = &hdc_ctrl->sessions[cmd->session_fd];

    if (session->owner != HDCDRV_SESSION_OWNER_PM) {
        hdcdrv_err("No right to get fid.\n");
        return HDCDRV_NO_PERMISSION;
    }

    cmd->fid = session->remote_fid;

    return HDCDRV_OK;
}

STATIC long hdcdrv_convert_id_from_vir_to_phy(u32 drv_cmd, union hdcdrv_cmd *cmd_data, u32 *vfid)
{
    long ret = HDCDRV_OK;

#ifndef HDC_ENV_DEVICE
    int *p_devid = NULL;
    int local_devid = 0;
    switch (drv_cmd) {
        case HDCDRV_CMD_GET_PEER_DEV_ID:
        case HDCDRV_CMD_SERVER_CREATE:
        case HDCDRV_CMD_SERVER_DESTROY:
        case HDCDRV_CMD_ACCEPT:
        case HDCDRV_CMD_CONNECT:
        case HDCDRV_CMD_ALLOC_MEM:
        case HDCDRV_CMD_DMA_MAP:
        case HDCDRV_CMD_DMA_REMAP:
            p_devid = &cmd_data->cmd_com.dev_id;
            break;
        case HDCDRV_CMD_EPOLL_ALLOC_FD:
            p_devid = &local_devid;
            break;
        case HDCDRV_CMD_EPOLL_CTL:
            p_devid = hdcdrv_epoll_get_dev_id_ptr(cmd_data);
            if (p_devid == NULL) {
                return HDCDRV_OK;
            }
            break;
        default:
            return ret;
    }
    ret = hdcdrv_container_vir_to_phs_devid((u32)(*p_devid), (u32 *)p_devid, vfid);
#endif

    return ret;
}

STATIC long hdcdrv_operation(struct hdcdrv_ctx *ctx, u32 drv_cmd, union hdcdrv_cmd *cmd_data,
    bool *copy_to_user_flag, u32 fid)
{
    long ret = HDCDRV_OK;

    switch (drv_cmd) {
        case HDCDRV_CMD_GET_PEER_DEV_ID:
            ret = hdcdrv_cmd_get_peer_dev_id(&cmd_data->get_peer_dev_id);
            *copy_to_user_flag = true;
            break;
        case HDCDRV_CMD_CONFIG:
            ret = hdcdrv_config(&cmd_data->config);
            *copy_to_user_flag = true;
            break;
        case HDCDRV_CMD_SET_SERVICE_LEVEL:
            ret = hdcdrv_set_service_level(&cmd_data->set_level);
            break;
        case HDCDRV_CMD_SERVER_CREATE:
            ret = hdcdrv_server_create(ctx, &cmd_data->server_create, fid);
            break;
        case HDCDRV_CMD_SERVER_DESTROY:
            ret = hdcdrv_server_destroy(ctx, &cmd_data->server_destroy, fid);
            break;
        case HDCDRV_CMD_ACCEPT:
            ret = hdcdrv_accept(&cmd_data->accept, fid);
            *copy_to_user_flag = true;
            break;
        case HDCDRV_CMD_CONNECT:
            ret = hdcdrv_connect(&cmd_data->conncet, fid);
            *copy_to_user_flag = true;
            break;
        case HDCDRV_CMD_CLOSE:
            ret = hdcdrv_close(&cmd_data->close, HDCDRV_CLOSE_TYPE_USER);
            break;
        case HDCDRV_CMD_SEND:
            ret = hdcdrv_send(&cmd_data->send, HDCDRV_MODE_USER);
            break;
        case HDCDRV_CMD_RECV_PEEK:
            ret = hdcdrv_recv_peek(&cmd_data->recv_peek);
            *copy_to_user_flag = true;
            break;
        case HDCDRV_CMD_RECV:
            ret = hdcdrv_recv(&cmd_data->recv, HDCDRV_MODE_USER);
            *copy_to_user_flag = true;
            break;
        default:
            hdcdrv_err_limit("drv_cmd is illegal. (cmd=%u)\n", drv_cmd);
            return HDCDRV_PARA_ERR;
    }

    return ret;
}

STATIC long hdcdrv_cfg_operation(struct hdcdrv_ctx *ctx, u32 drv_cmd, union hdcdrv_cmd *cmd_data,
    bool *copy_to_user_flag, u32 fid)
{
    long ret = HDCDRV_OK;

    switch (drv_cmd) {
        case HDCDRV_CMD_SET_SESSION_OWNER:
            ret = hdcdrv_set_session_owner(ctx, &cmd_data->set_owner, fid);
            break;
        case HDCDRV_CMD_GET_SESSION_ATTR:
            ret = hdcdrv_get_session_attr(&cmd_data->get_session_attr);
            *copy_to_user_flag = true;
            break;
        case HDCDRV_CMD_SET_SESSION_TIMEOUT:
            ret = hdcdrv_set_session_timeout(&cmd_data->set_session_timeout);
            break;
        case HDCDRV_CMD_GET_SESSION_UID:
            ret = hdcdrv_get_uid_state(&cmd_data->get_uid_stat);
            *copy_to_user_flag = true;
            break;
        case HDCDRV_CMD_GET_PAGE_SIZE:
            ret = hdcdrv_get_page_size(&cmd_data->get_page_size);
            *copy_to_user_flag = true;
            break;
        case HDCDRV_CMD_GET_SESSION_INFO:
            ret = hdcdrv_get_session_info(&cmd_data->get_session_info);
            *copy_to_user_flag = true;
            break;
        default:
            hdcdrv_err_limit("drv_cmd is illegal. (cmd=%u)\n", drv_cmd);
            return HDCDRV_PARA_ERR;
    }

    return ret;
}

STATIC long hdcdrv_fast_operation(struct hdcdrv_ctx *ctx, u32 drv_cmd,
    union hdcdrv_cmd *cmd_data, bool *copy_to_user_flag)
{
    long ret = HDCDRV_OK;

    switch (drv_cmd) {
        case HDCDRV_CMD_ALLOC_MEM:
            ret = hdcdrv_fast_alloc_mem(ctx, &cmd_data->alloc_mem);
            break;
        case HDCDRV_CMD_FREE_MEM:
            ret = hdcdrv_fast_free_mem((void *)ctx, &cmd_data->free_mem);
            *copy_to_user_flag = true;
            break;
        case HDCDRV_CMD_FAST_SEND:
            ret = hdcdrv_fast_send(&cmd_data->fast_send);
            break;
        case HDCDRV_CMD_FAST_RECV:
            ret = hdcdrv_fast_recv(&cmd_data->fast_recv);
            *copy_to_user_flag = true;
            break;
        case HDCDRV_CMD_DMA_MAP:
            ret = hdcdrv_fast_dma_map(&cmd_data->dma_map);
            break;
        case HDCDRV_CMD_DMA_UNMAP:
            ret = hdcdrv_fast_dma_unmap(&cmd_data->dma_unmap);
            break;
        case HDCDRV_CMD_DMA_REMAP:
            ret = hdcdrv_fast_dma_remap(&cmd_data->dma_remap);
            break;
#ifdef CFG_FEATURE_HDC_REG_MEM
        case HDCDRV_CMD_REGISTER_MEM:
            ret = hdcdrv_fast_register_mem(ctx, &cmd_data->register_mem);
            break;
        case HDCDRV_CMD_UNREGISTER_MEM:
            ret = hdcdrv_fast_unregister_mem((void *)ctx, &cmd_data->unregister_mem);
            break;
        case HDCDRV_CMD_WAIT_MEM:
            ret = hdcdrv_fast_wait_mem(&cmd_data->wait_mem);
            *copy_to_user_flag = true;
            break;
#endif
        default:
            hdcdrv_err_limit("drv_cmd is illegal. (cmd=%u)\n", drv_cmd);
            return HDCDRV_PARA_ERR;
    }

    return ret;
}

long hdcdrv_ioctl_com(struct hdcdrv_ctx *ctx, unsigned int cmd, union hdcdrv_cmd *cmd_data,
    bool *copy_flag, u32 fid)
{
    u32 drv_cmd = _IOC_NR(cmd);
    long ret;

    if (_IOC_TYPE(cmd) != HDCDRV_CMD_MAGIC) {
        hdcdrv_err("Command type is error. (cmd=%x)\n", cmd);
        return HDCDRV_PARA_ERR;
    }

    if (drv_cmd < HDCDRV_CMD_SET_SESSION_OWNER) {
        ret = hdcdrv_operation(ctx, drv_cmd, cmd_data, copy_flag, fid);
    } else if (drv_cmd < HDCDRV_CMD_ALLOC_MEM) {
        ret = hdcdrv_cfg_operation(ctx, drv_cmd, cmd_data, copy_flag, fid);
    } else if (drv_cmd < HDCDRV_CMD_EPOLL_ALLOC_FD) {
        ret = hdcdrv_fast_operation(ctx, drv_cmd, cmd_data, copy_flag);
    } else {
        ret = hdcdrv_epoll_operation(ctx, drv_cmd, cmd_data, copy_flag, fid);
    }

    return ret;
}

STATIC long hdcdrv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)((uintptr_t)arg);
    struct hdcdrv_ctx *ctx = NULL;
    u32 drv_cmd = _IOC_NR(cmd);
    bool copy_flag = false;
    union hdcdrv_cmd cmd_data;
    u32 vfid = HDCDRV_DEFAULT_PM_FID;
    long ret;

    if ((file == NULL) || (argp == NULL)) {
        hdcdrv_err("Input pararmeter is error.\n");
        return HDCDRV_PARA_ERR;
    }

    if (hdcdrv_get_running_status() == HDCDRV_RUNNING_SUSPEND) {
        return HDCDRV_DEVICE_NOT_READY;
    }

    if (copy_from_user(&cmd_data, argp, sizeof(cmd_data)) != 0) {
        hdcdrv_err("Calling copy_from_user failed. (cmd=%u)\n", drv_cmd);
        return HDCDRV_COPY_FROM_USER_FAIL;
    }

    if (hdcdrv_convert_id_from_vir_to_phy(drv_cmd, &cmd_data, &vfid) != HDCDRV_OK) {
        hdcdrv_err("Convert virtual id to physical id failed. (drv_cmd=%u)\n", drv_cmd);
        return HDCDRV_CONV_FAILED;
    }

    ctx = (struct hdcdrv_ctx *)file->private_data;
    cmd_data.cmd_com.pid = hdcdrv_rebuild_pid((u32)cmd_data.cmd_com.dev_id, vfid, hdcdrv_get_pid());

    ret = hdcdrv_ioctl_com(ctx, cmd, &cmd_data, &copy_flag, vfid);
    if (ret != HDCDRV_OK) {
        return ret;
    }

    if ((ret == HDCDRV_OK) && copy_flag) {
        if (copy_to_user(argp, &cmd_data, sizeof(cmd_data)) != 0) {
            hdcdrv_err("copy_to_user failed. (drv_cmd=%d)\n", drv_cmd);
            return HDCDRV_COPY_TO_USER_FAIL;
        }
    }

    return HDCDRV_OK;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
STATIC vm_fault_t hdcdrv_svm_vmf_fault_host(struct vm_fault *vmf)
{
    /* Page fault interrupt mapping pa is not supported for hdc mmap */
    hdcdrv_warn("hdc mmap not supported page fault");
    return HDCDRV_FAULT_ERROR;
}
#endif

int hdcdrv_svm_vm_fault_host(struct vm_area_struct *vma, struct vm_fault *vmf)
{
    /* Page fault interrupt mapping pa is not supported for hdc mmap */
    hdcdrv_warn("hdc mmap not supported page fault");
    return (int)HDCDRV_FAULT_ERROR;
}

STATIC void hdcdrv_svm_mmap_init_vm_struct(struct vm_operations_struct *ops_managed)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    ops_managed->fault = hdcdrv_svm_vmf_fault_host;
#else
    ops_managed->fault = hdcdrv_svm_vm_fault_host;
#endif
}

/* Avoid the user process calling mlockall (MCL_FUTURE) to establish the page table in advance,
   which will cause our subsequent page table creation to fail */
STATIC int hdcdrv_mmap(struct file *filep, struct vm_area_struct *vma)
{
    int ret;

    ret = hdcdrv_mmap_param_check(filep, vma);
    if (ret != 0) {
        hdcdrv_err("mmap param check failed. (ret=%d)\n", ret);
        return -EINVAL;
    }

    vma->vm_private_data = filep->private_data;
    hdcdrv_svm_mmap_init_vm_struct(&hdcdrv_vm_ops_managed);
    vma->vm_ops = &hdcdrv_vm_ops_managed;

    return HDCDRV_OK;
}

void hdcdrv_set_debug_mode(int flag)
{
    mutex_lock(&hdc_ctrl->mutex);
    if (flag != 0) {
        hdc_ctrl->debug_state.valid = HDCDRV_VALID;
        hdcdrv_info("Step into adjustment mode.\n");
    } else {
        hdc_ctrl->debug_state.valid = HDCDRV_INVALID;
        hdcdrv_info("Step into adjustment mode.\n");
    }
    hdc_ctrl->debug_state.pid = (long long)hdcdrv_get_pid();
    mutex_unlock(&hdc_ctrl->mutex);

    return;
}
EXPORT_SYMBOL(hdcdrv_set_debug_mode);

STATIC int hdcdrv_open(struct inode *node, struct file *file)
{
    struct hdcdrv_ctx *ctx = hdcdrv_alloc_ctx();
    if (ctx == NULL) {
        hdcdrv_err("Calling kzalloc failed.\n");
        return HDCDRV_MEM_ALLOC_FAIL;
    }

    file->private_data = ctx;

    return HDCDRV_OK;
}

STATIC void hdcdrv_release_close_session(struct hdcdrv_ctx *ctx)
{
    int ret = 0;
    struct hdcdrv_cmd_close close_cmd;
    struct hdcdrv_session *session = ctx->session;

    /* session free */
    if (ctx->session != NULL) {
        hdcdrv_info("Release session. (dev=%d; task_pid=%llu; local session_id=%d; remote_id=%d; session_status=%d)\n",
            ctx->dev_id, ctx->pid, ctx->session_fd, session->remote_session_fd,
            hdcdrv_get_session_status(session));

        close_cmd.session = ctx->session_fd;
        close_cmd.pid = ctx->pid;
        close_cmd.unique_val = session->unique_val;
        ret = (int)hdcdrv_close(&close_cmd, HDCDRV_CLOSE_TYPE_RELEASE);
        if (ret != HDCDRV_OK) {
            hdcdrv_warn("Close session not success. (dev=%d; task_pid=%llu; local_id=%d; remote_id=%d; status=%d)\n",
                ctx->dev_id, ctx->pid, ctx->session_fd, session->remote_session_fd,
                hdcdrv_get_session_status(session));
        }
        ctx->session = NULL;
    }
}

STATIC void hdcdrv_release_destroy_server(struct hdcdrv_ctx *ctx)
{
    int ret = 0;
    struct hdcdrv_cmd_server_destroy destroy_cmd;

    /* service free */
    if (ctx->service != NULL) {
        hdcdrv_info("Release server. (task_pid=%llu; dev_id=%d; service_type=\"%s\")\n",
            ctx->service->listen_pid, ctx->dev_id, hdcdrv_sevice_str(ctx->service_type));

        destroy_cmd.dev_id = ctx->dev_id;
        destroy_cmd.service_type = ctx->service_type;
        destroy_cmd.pid = ctx->pid;
        ret = (int)hdcdrv_server_destroy(ctx, &destroy_cmd, ctx->service->fid);
        if (ret != HDCDRV_OK) {
            hdcdrv_warn("Release server. (task_pid=%llu; dev_id=%d; service_type=\"%s\")\n",
                ctx->pid, ctx->dev_id, hdcdrv_sevice_str(ctx->service_type));
        }

        ctx->service = NULL;
    }
}

STATIC int hdcdrv_release(struct inode *node, struct file *file)
{
    struct hdcdrv_ctx *ctx = file->private_data;

    if (ctx == NULL) {
        hdcdrv_err("Input pararmeter is error. (task pid=%llu)", hdcdrv_get_pid());
        return HDCDRV_PARA_ERR;
    }

    hdcdrv_release_by_ctx(ctx);
    hdcdrv_free_ctx(ctx);
    file->private_data = NULL;

    if (hdc_ctrl->debug_state.pid == hdcdrv_get_pid()) {
        hdcdrv_set_debug_mode(HDCDRV_INVALID);
    }

    return HDCDRV_OK;
}

struct hdcdrv_ctx *hdcdrv_alloc_ctx(void)
{
    struct hdcdrv_ctx *ctx = kzalloc(sizeof(struct hdcdrv_ctx), GFP_KERNEL | __GFP_ACCOUNT);
    if (ctx != NULL) {
        ctx->service = NULL;
        ctx->session = NULL;
        ctx->epfd = NULL;

        spin_lock_init(&ctx->ctx_fmem.mem_lock);
        INIT_LIST_HEAD(&ctx->ctx_fmem.mlist.list);
    }

    return ctx;
}

void hdcdrv_free_ctx(const struct hdcdrv_ctx *ctx)
{
    kfree(ctx);
    ctx = NULL;
}

void hdcdrv_release_by_ctx(struct hdcdrv_ctx *ctx)
{
    hdcdrv_release_close_session(ctx);
    hdcdrv_release_destroy_server(ctx);
    hdcdrv_release_free_mem(&ctx->ctx_fmem);
    hdcdrv_epoll_recycle_fd(ctx);
}

/* The following interface is provided for the kernel to call hdc */
bool hdcdrv_is_service_init(unsigned int service_type)
{
    if (service_type >= HDCDRV_SUPPORT_MAX_SERVICE) {
        return false;
    }

    if (hdc_ctrl == NULL) {
        return false;
    }

    return true;
}
EXPORT_SYMBOL(hdcdrv_is_service_init);

/* for ide type, set level to 1 */
long hdcdrv_kernel_set_service_level(int service_type, int level)
{
    struct hdcdrv_cmd_set_service_level cmd;

    cmd.service_type = service_type;
    cmd.level = level;

    return hdcdrv_set_service_level(&cmd);
}
EXPORT_SYMBOL(hdcdrv_kernel_set_service_level);

void hdcdrv_set_segment(int segment)
{
    hdc_ctrl->segment = segment;
    hdcdrv_info("Set segment success. (hdc_segment=%d)\n", hdc_ctrl->segment);
}

int hdcdrv_get_segment(void)
{
    return hdcdrv_mem_block_capacity();
}
EXPORT_SYMBOL(hdcdrv_get_segment);

void hdcdrv_set_peer_dev_id(int dev_id, int peer_dev_id)
{
    hdc_ctrl->devices[dev_id].peer_dev_id = peer_dev_id;
}

int hdcdrv_get_peer_dev_id(int dev_id)
{
    return hdc_ctrl->devices[dev_id].peer_dev_id;
}

long hdcdrv_kernel_server_create(int dev_id, int service_type)
{
    struct hdcdrv_cmd_server_create cmd;

    cmd.dev_id = dev_id;
    cmd.service_type = service_type;
    cmd.pid = hdcdrv_get_pid();

    return hdcdrv_server_create(HDCDRV_KERNEL_WITHOUT_CTX, &cmd, HDCDRV_DEFAULT_PM_FID);
}
EXPORT_SYMBOL(hdcdrv_kernel_server_create);

long hdcdrv_kernel_server_destroy(int dev_id, int service_type)
{
    struct hdcdrv_cmd_server_destroy cmd;

    cmd.dev_id = dev_id;
    cmd.service_type = service_type;
    cmd.pid = hdcdrv_get_pid();

    return hdcdrv_server_destroy(HDCDRV_KERNEL_WITHOUT_CTX, &cmd, HDCDRV_DEFAULT_PM_FID);
}
EXPORT_SYMBOL(hdcdrv_kernel_server_destroy);

long hdcdrv_kernel_accept(int dev_id, int service_type, int *session, const char *session_id)
{
    struct hdcdrv_cmd_set_session_owner set_pid_cmd;
    struct hdcdrv_cmd_accept cmd;
    long ret;
    long res;

    (void)session_id;
    if (session == NULL) {
        return HDCDRV_PARA_ERR;
    }

    cmd.dev_id = dev_id;
    cmd.service_type = service_type;
    cmd.pid = hdcdrv_get_pid();

    ret = hdcdrv_accept(&cmd, HDCDRV_DEFAULT_PM_FID);
    if (ret == HDCDRV_OK) {
        *session = cmd.session;
        set_pid_cmd.session = cmd.session;
        set_pid_cmd.pid = hdcdrv_get_pid();
        set_pid_cmd.dev_id = dev_id;

        res = hdcdrv_set_session_owner(HDCDRV_KERNEL_WITHOUT_CTX, &set_pid_cmd, HDCDRV_DEFAULT_PM_FID);
        if (res != HDCDRV_OK) {
            hdcdrv_err("Set owner failed. (dev=%d; session=%d)\n", dev_id, cmd.session);
        }
    }

    return ret;
}
EXPORT_SYMBOL(hdcdrv_kernel_accept);

long hdcdrv_kernel_connect(int dev_id, int service_type, int *session, const char *session_id)
{
    struct hdcdrv_cmd_set_session_owner set_pid_cmd;
    struct hdcdrv_cmd_connect cmd;
    long ret;
    long res;

    (void)session_id;
    if (session == NULL) {
        return HDCDRV_PARA_ERR;
    }

    cmd.dev_id = dev_id;
    cmd.service_type = service_type;
    cmd.peer_pid = HDCDRV_INVALID_PEER_PID;
    cmd.pid = hdcdrv_get_pid();

    ret = hdcdrv_connect(&cmd, HDCDRV_DEFAULT_PM_FID);
    if (ret == HDCDRV_OK) {
        *session = cmd.session;
        set_pid_cmd.session = cmd.session;
        set_pid_cmd.pid = hdcdrv_get_pid();
        set_pid_cmd.dev_id = dev_id;

        res = hdcdrv_set_session_owner(HDCDRV_KERNEL_WITHOUT_CTX, &set_pid_cmd, HDCDRV_DEFAULT_PM_FID);
        if (res != HDCDRV_OK) {
            hdcdrv_err("Set owner failed. (dev=%d; session=%d)\n", dev_id, cmd.session);
        }
    }

    return ret;
}
EXPORT_SYMBOL(hdcdrv_kernel_connect);

long hdcdrv_kernel_close(int session, const char *session_id)
{
    struct hdcdrv_cmd_close cmd;

    (void)session_id;
    if ((session >= HDCDRV_REAL_MAX_SESSION) || (session < 0)) {
        hdcdrv_err("Input pararmeter is error. (session_fd=%d)\n", session);
        return HDCDRV_PARA_ERR;
    }

    cmd.session = session;
    cmd.pid = hdcdrv_get_pid();
    cmd.unique_val = hdc_ctrl->sessions[session].unique_val;

    return hdcdrv_close(&cmd, HDCDRV_CLOSE_TYPE_KERNEL);
}
EXPORT_SYMBOL(hdcdrv_kernel_close);

long hdcdrv_kernel_send_timeout(int session, const char *session_id, void *buf, int len, int timeout)
{
    struct hdcdrv_session *ksession = NULL;
    struct hdcdrv_cmd_send cmd;

    (void)session_id;
    if ((buf == NULL)) {
        return HDCDRV_PARA_ERR;
    }

    if ((session < 0) || (session >= HDCDRV_REAL_MAX_SESSION)) {
        hdcdrv_err_limit("Input pararmeter is error. (session_fd=%d)\n", session);
        return HDCDRV_PARA_ERR;
    }

    ksession = &hdc_ctrl->sessions[session];

    cmd.session = session;
    cmd.dev_id = ksession->dev_id;
    cmd.src_buf = buf;
    cmd.len = len;
    cmd.wait_flag = hdcdrv_get_wait_flag(timeout);
    cmd.timeout = (unsigned int)timeout;
    cmd.pool_buf = NULL;
    cmd.pid = hdcdrv_get_pid();

    return hdcdrv_send(&cmd, HDCDRV_MODE_KERNEL);
}
EXPORT_SYMBOL(hdcdrv_kernel_send_timeout);

long hdcdrv_kernel_send(int session, const char *session_id, void *buf, int len)
{
    /* send block */
    return hdcdrv_kernel_send_timeout(session, session_id, buf, len, -1);
}
EXPORT_SYMBOL(hdcdrv_kernel_send);

long hdcdrv_kernel_recv_peek_timeout(int session, const char *session_id, int *len, int timeout)
{
    struct hdcdrv_session *ksession = NULL;
    struct hdcdrv_cmd_recv_peek cmd;
    long ret;

    (void)session_id;
    if (len == NULL) {
        return HDCDRV_PARA_ERR;
    }

    if ((session < 0) || (session >= HDCDRV_REAL_MAX_SESSION)) {
        hdcdrv_err_limit("Input pararmeter is error. (session_fd=%d)\n", session);
        return HDCDRV_PARA_ERR;
    }

    ksession = &hdc_ctrl->sessions[session];

    cmd.session = session;
    cmd.dev_id = ksession->dev_id;
    cmd.wait_flag = hdcdrv_get_wait_flag(timeout);
    cmd.timeout = (unsigned int)timeout;
    cmd.pid = hdcdrv_get_pid();
    cmd.group_flag = 0;

    ret = hdcdrv_recv_peek(&cmd);
    if (ret == HDCDRV_OK) {
        *len = cmd.len;
    }

    return ret;
}
EXPORT_SYMBOL(hdcdrv_kernel_recv_peek_timeout);

long hdcdrv_kernel_recv_peek(int session, const char *session_id, int *len)
{
    /* recv peek block */
    return hdcdrv_kernel_recv_peek_timeout(session, session_id, len, -1);
}
EXPORT_SYMBOL(hdcdrv_kernel_recv_peek);

long hdcdrv_kernel_recv(int session, const char *session_id, void *buf, int len, int *out_len)
{
    struct hdcdrv_session *ksession = NULL;
    struct hdcdrv_cmd_recv cmd;
    long ret;

    (void)session_id;
    if ((buf == NULL) || (out_len == NULL)) {
        return HDCDRV_PARA_ERR;
    }

    if ((session < 0) || (session >= HDCDRV_REAL_MAX_SESSION)) {
        hdcdrv_err_limit("Input pararmeter is error. (session_fd=%d)\n", session);
        return HDCDRV_PARA_ERR;
    }

    ksession = &hdc_ctrl->sessions[session];

    cmd.session = session;
    cmd.dev_id = ksession->dev_id;
    cmd.dst_buf = buf;
    cmd.len = len;
    cmd.pid = hdcdrv_get_pid();
    cmd.buf_count = 1;          // kernel_recv use normal recv, the buf_cout is 1

    ret = hdcdrv_recv(&cmd, HDCDRV_MODE_KERNEL);
    if (ret == HDCDRV_OK) {
        *out_len = cmd.out_len;
    }

    return ret;
}
EXPORT_SYMBOL(hdcdrv_kernel_recv);

long hdcdrv_kernel_get_session_attr(int session, int attr, int *value)
{
    struct hdcdrv_cmd_get_session_attr cmd;
    struct hdcdrv_session *ksession = NULL;
    long ret;

    if (value == NULL) {
        hdcdrv_err("Input pararmeter is error.\n");
        return HDCDRV_PARA_ERR;
    }

    if ((session < 0) || (session >= HDCDRV_REAL_MAX_SESSION)) {
        hdcdrv_err("Input pararmeter is error. (session_fd=%d)\n", session);
        return HDCDRV_PARA_ERR;
    }

    ksession = &hdc_ctrl->sessions[session];

    cmd.cmd_type = attr;
    cmd.session = session;
    cmd.dev_id = ksession->dev_id;
    cmd.pid = hdcdrv_get_pid();

    ret = hdcdrv_get_session_attr(&cmd);
    if (ret == HDCDRV_OK) {
        *value = cmd.output;
    }

    return ret;
}

long hdcdrv_kernel_get_session_run_env(int session, const char *session_id, int *run_env)
{
    (void)session_id;
    return hdcdrv_kernel_get_session_attr(session, HDCDRV_SESSION_ATTR_RUN_ENV, run_env);
}
EXPORT_SYMBOL(hdcdrv_kernel_get_session_run_env);

long hdcdrv_kernel_get_session_vfid(int session, int *value)
{
    return hdcdrv_kernel_get_session_attr(session, HDCDRV_SESSION_ATTR_VFID, value);
}
EXPORT_SYMBOL(hdcdrv_kernel_get_session_vfid);

long hdcdrv_kernel_get_session_local_create_pid(int session, int *value)
{
    return hdcdrv_kernel_get_session_attr(session, HDCDRV_SESSION_ATTR_LOCAL_CREATE_PID, value);
}
EXPORT_SYMBOL(hdcdrv_kernel_get_session_local_create_pid);

long hdcdrv_kernel_get_session_peer_create_pid(int session, int *value)
{
    return hdcdrv_kernel_get_session_attr(session, HDCDRV_SESSION_ATTR_PEER_CREATE_PID, value);
}
EXPORT_SYMBOL(hdcdrv_kernel_get_session_peer_create_pid);

void hdcdrv_session_notify_register(int service_type, struct hdcdrv_session_notify *notify)
{
    if ((service_type >= HDCDRV_SUPPORT_MAX_SERVICE) || (service_type < 0)) {
        hdcdrv_err("Input pararmeter is error. (service_type=\"%s\")\n", hdcdrv_sevice_str(service_type));
        return ;
    }

    g_session_notify[service_type].connect_notify = notify->connect_notify;
    g_session_notify[service_type].close_notify = notify->close_notify;
    g_session_notify[service_type].data_in_notify = notify->data_in_notify;

    return;
}
EXPORT_SYMBOL(hdcdrv_session_notify_register);

void hdcdrv_session_notify_unregister(int service_type)
{
    if ((service_type >= HDCDRV_SUPPORT_MAX_SERVICE) || (service_type < 0)) {
        hdcdrv_err("Input pararmeter is error. (service_type=\"%s\")\n", hdcdrv_sevice_str(service_type));
        return ;
    }

    g_session_notify[service_type].connect_notify = NULL;
    g_session_notify[service_type].close_notify = NULL;
    g_session_notify[service_type].data_in_notify = NULL;
}
EXPORT_SYMBOL(hdcdrv_session_notify_unregister);

/* value 0 means rx list is not full, value 1 means rx list is full */
int hdcdrv_get_session_rx_list_status(int session_fd, int *value)
{
    if (value == NULL) {
        hdcdrv_err("Input pararmeter is error.\n");
        return HDCDRV_PARA_ERR;
    }

    if (session_fd < 0 || session_fd >= HDCDRV_REAL_MAX_SESSION) {
        hdcdrv_err("session_fd is illegal. (session_fd=%d)\n", session_fd);
        return HDCDRV_PARA_ERR;
    }

    *value = hdcdrv_session_rx_list_is_full(session_fd, hdc_ctrl->sessions[session_fd].msg_chan->type);

    return HDCDRV_OK;
}
EXPORT_SYMBOL(hdcdrv_get_session_rx_list_status);

void hdcdrv_register_symbol_from_tsdrv(struct hdcdrv_register_symbol *module_symbol)
{
    if (module_symbol == NULL) {
        hdcdrv_err("invalid arg.\n");
        return;
    }

    write_lock(&g_symbol_lock);
    if (g_hdcdrv_register_symbol.module_ptr != NULL) {
        hdcdrv_err("The module has registered symbols.\n");
        write_unlock(&g_symbol_lock);
        return;
    }

    g_hdcdrv_register_symbol.module_ptr = module_symbol->module_ptr;
    g_hdcdrv_register_symbol.wake_up_context_status = module_symbol->wake_up_context_status;
    write_unlock(&g_symbol_lock);
    return;
}
EXPORT_SYMBOL(hdcdrv_register_symbol_from_tsdrv);

void hdcdrv_unregister_symbol_from_tsdrv(void)
{
    write_lock(&g_symbol_lock);
    g_hdcdrv_register_symbol.module_ptr = NULL;
    g_hdcdrv_register_symbol.wake_up_context_status = NULL;
    write_unlock(&g_symbol_lock);
    return;
}
EXPORT_SYMBOL(hdcdrv_unregister_symbol_from_tsdrv);

STATIC void hdcdrv_guard_work(struct work_struct *p_work)
{
    int i, j;
    struct hdcdrv_msg_chan *msg_chan = NULL;
    struct delayed_work *delayed_work = container_of(p_work, struct delayed_work, work);

    for (i = 0; i < hdcdrv_get_max_support_dev(); i++) {
        if (hdc_ctrl->devices[i].valid != HDCDRV_VALID) {
            continue;
        }

        for (j = 0; j < HDCDRV_SUPPORT_MAX_DEV_MSG_CHAN; j++) {
            msg_chan = hdc_ctrl->devices[i].msg_chan[j];
            if (msg_chan == NULL) {
                continue;
            }

            hdcdrv_tx_finish_task_check(msg_chan);
            hdcdrv_rx_msg_task_check(msg_chan);
            hdcdrv_rx_msg_notify_task_check(msg_chan);
        }
    }

    (void)schedule_delayed_work(delayed_work, 1 * HZ);
}

STATIC void hdcdrv_close_unknow_session_work(struct work_struct *p_work)
{
    int ret = 0;
    struct hdcdrv_cmd_close close_cmd;
    struct hdcdrv_session *session = container_of(p_work, struct hdcdrv_session, close_unknow_session.work);

    hdcdrv_delay_work_flag_clear(session, HDCDRV_DELAY_UNKNOWN_SESSION_BIT);
    if (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_CONN) {
        hdcdrv_info("Hdcdrv close unknow session. (dev=%d; service_type=\"%s\";"
            "local_session_fd=%d; remote_session_fd=%d; set_owner_flag=%d)\n",
            session->dev_id, hdcdrv_sevice_str(session->service_type),
            session->local_session_fd, session->remote_session_fd, session->pid_flag);

        close_cmd.session = session->local_session_fd;
        close_cmd.pid = session->owner_pid;
        close_cmd.unique_val = session->unique_val;

        ret = (int)hdcdrv_close(&close_cmd, (int)HDCDRV_CLOSE_TYPE_NOT_SET_OWNER);
        if (ret != HDCDRV_OK) {
            hdcdrv_warn("Calling hdcdrv_close not success. (dev_id=%d; ret=%d)\n", session->dev_id, ret);
        }
    }
}

#ifdef CFG_FEATURE_VFIO
STATIC void hdcdrv_session_work_list_free(struct hdcdrv_session_work *s_work)
{
    struct hdcdrv_session_work_node *entry[HDCDRV_LIST_CACHE] = {NULL};
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    int count;
    int i;

    while (!list_empty_careful(&s_work->root_list)) {
        count = 0;
        spin_lock_bh(&s_work->lock);
        list_for_each_safe(pos, n, &s_work->root_list) {
            entry[count] = list_entry(pos, struct hdcdrv_session_work_node, list);
            list_del(&entry[count]->list);
            count++;
            if (count >= HDCDRV_LIST_CACHE) {
                break;
            }
        }
        spin_unlock_bh(&s_work->lock);

        for (i = 0; i < count; i++) {
            vhdch_free_mem_vm(entry[i]->dev_id, entry[i]->fid, entry[i]->buf);
            hdcdrv_dma_unmap_guest_page(entry[i]->dev_id, entry[i]->fid, entry[i]->dma_sgt);

            kfree(entry[i]);
            entry[i] = NULL;
        }
    }
}

STATIC void hdcdrv_session_work_task(struct work_struct *p_work)
{
    struct hdcdrv_session *session = container_of(p_work, struct hdcdrv_session, session_work.swork);

    hdcdrv_session_work_list_free(&session->session_work);
}

STATIC void hdcdrv_session_work_init(struct hdcdrv_session_work *s_work)
{
    INIT_WORK(&s_work->swork, hdcdrv_session_work_task);
    spin_lock_init(&s_work->lock);
    INIT_LIST_HEAD(&s_work->root_list);
}

void hdcdrv_session_work_free(struct hdcdrv_session_work *s_work)
{
    cancel_work_sync(&s_work->swork);
    hdcdrv_session_work_list_free(s_work);
}
#endif

int hdcdrv_add_ctrl_msg_chan_to_dev(u32 dev_id, void *chan)
{
    struct hdcdrv_dev *hdc_dev = NULL;

    if (dev_id >= (u32)hdcdrv_get_max_support_dev()) {
        hdcdrv_err("Input pararmeter is error. (dev_id=%u)\n", dev_id);
        return HDCDRV_PARA_ERR;
    }

    hdc_dev = &hdc_ctrl->devices[dev_id];
    hdc_dev->ctrl_msg_chan = chan;
    return HDCDRV_OK;
}

int hdcdrv_add_msg_chan_to_dev(u32 dev_id, void *chan)
{
    struct hdcdrv_dev *hdc_dev = NULL;
    struct hdcdrv_msg_chan *msg_chan = NULL;
    int i;

    if (dev_id >= (u32)hdcdrv_get_max_support_dev()) {
        hdcdrv_err("Input pararmeter is error. (dev_id=%u)\n", dev_id);
        return HDCDRV_PARA_ERR;
    }

    hdc_dev = &hdc_ctrl->devices[dev_id];

    mutex_lock(&hdc_dev->mutex);

    if ((hdc_dev->msg_chan_cnt >= (int)HDCDRV_SUPPORT_MAX_DEV_MSG_CHAN) || (hdc_dev->msg_chan_cnt < 0)) {
        mutex_unlock(&hdc_dev->mutex);
        hdcdrv_err("Parameter msg_chan_cnt out of range. (msg_chan_cnt=%d)\n", hdc_dev->msg_chan_cnt);
        return HDCDRV_PARA_ERR;
    }

    msg_chan = hdc_dev->msg_chan[hdc_dev->msg_chan_cnt];
    if (msg_chan == NULL) {
        msg_chan = (struct hdcdrv_msg_chan *)hdcdrv_kvmalloc(sizeof(struct hdcdrv_msg_chan));
        if (msg_chan == NULL) {
            mutex_unlock(&hdc_dev->mutex);
            hdcdrv_err("Calling alloc failed. (dev=%u; size=%ld)\n", dev_id, sizeof(struct hdcdrv_msg_chan));
            return HDCDRV_MEM_ALLOC_FAIL;
        }

        if (memset_s(msg_chan, sizeof(struct hdcdrv_msg_chan), 0, sizeof(struct hdcdrv_msg_chan)) != 0) {
            mutex_unlock(&hdc_dev->mutex);
            hdcdrv_kvfree(msg_chan);
            msg_chan = NULL;
            hdcdrv_err("Calling memset_s failed.\n");
            return HDCDRV_SAFE_MEM_OP_FAIL;
        }

        mutex_init(&msg_chan->mutex);
    }

    if (hdc_dev->msg_chan_cnt == 0) {
        if (hdcdrv_init_mem_pool(hdc_dev->dev_id) != HDCDRV_OK) {
            mutex_unlock(&hdc_dev->mutex);
            hdcdrv_kvfree(msg_chan);
            msg_chan = NULL;
            return HDCDRV_DMA_MEM_ALLOC_FAIL;
        }
    }

    msg_chan->chan_id = hdc_dev->msg_chan_cnt;
    msg_chan->dev_id = dev_id;
    msg_chan->dev = hdc_dev->dev;
    msg_chan->dma_head = HDCDRV_DESC_QUEUE_DEPTH - 1;
    msg_chan->rx_head = HDCDRV_DESC_QUEUE_DEPTH - 1;
    msg_chan->submit_dma_head = HDCDRV_DESC_QUEUE_DEPTH - 1;
    msg_chan->chan = chan;
    msg_chan->session_cnt = 0;

    for (i = 0; i < HDCDRV_DESC_QUEUE_DEPTH; i++) {
        msg_chan->tx[i].buf = NULL;
        msg_chan->rx[i].buf = NULL;
    }

    msg_chan->rx_trigger_flag = HDCDRV_INVALID;
    msg_chan->dma_need_submit_flag = HDCDRV_INVALID;

    tasklet_init(&msg_chan->tx_finish_task, hdcdrv_tx_finish_notify_task, (uintptr_t)msg_chan);
    tasklet_init(&msg_chan->rx_task, hdcdrv_msg_chan_recv_task, (uintptr_t)msg_chan);

    /* only [vfio] and [mdev+sriov's vm] use workqueue, others use tasklet */
    if ((g_rx_notify_sched_mode == HDCDRV_RX_SCHED_WORK) || (devdrv_is_mdev_vm_boot_mode(dev_id) == true)) {
        msg_chan->rx_workqueue = create_singlethread_workqueue(HDCDRV_RX_MSG_NORIFY_WORK_NAME);
        if (msg_chan->rx_workqueue == NULL) {
            mutex_unlock(&hdc_dev->mutex);
            hdcdrv_err("Calling create_singlethread_workqueue failed. (name=\"%s\")\n", HDCDRV_RX_MSG_NORIFY_WORK_NAME);
            hdcdrv_uninit_mem_pool(hdc_dev->dev_id);
            hdcdrv_kvfree(msg_chan);
            msg_chan = NULL;
            return HDCDRV_MEM_ALLOC_FAIL;
        }
        INIT_WORK(&msg_chan->rx_notify_work, hdcdrv_rx_msg_notify_work);
    } else {
        tasklet_init(&msg_chan->rx_notify_task, hdcdrv_rx_msg_notify_task, (uintptr_t)msg_chan);
    }

    if ((u32)msg_chan->chan_id < hdc_dev->normal_chan_num) {
        msg_chan->type = HDCDRV_MSG_CHAN_TYPE_NORMAL;
    } else {
        msg_chan->type = HDCDRV_MSG_CHAN_TYPE_FAST;
    }

    msg_chan->data_type = hdcdrv_get_dma_data_type(msg_chan->type, msg_chan->chan_id);
    msg_chan->wait_mem_list.next = NULL;
    msg_chan->wait_mem_list.prev = NULL;

    init_waitqueue_head(&msg_chan->send_wait);

    (void)hdcdrv_set_msg_chan_priv(chan, msg_chan);

    hdc_dev->msg_chan[msg_chan->chan_id] = msg_chan;
    wmb();
    hdc_dev->msg_chan_cnt++;
    mutex_unlock(&hdc_dev->mutex);

    return HDCDRV_OK;
}

void hdcdrv_service_res_uninit(struct hdcdrv_service *service, int server_type)
{
    struct hdcdrv_serv_list_node *node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    if (hdc_ctrl->service_attr[server_type].service_scope == HDCDRV_SERVICE_SCOPE_GLOBAL) {
        return;
    }

    if (!list_empty_careful(&service->serv_list)) {
        list_for_each_safe(pos, n, &service->serv_list) {
            node = list_entry(pos, struct hdcdrv_serv_list_node, list);
            list_del(&node->list);
            kfree(node);
            node = NULL;
        }
    }

    return;
}

void hdcdrv_service_init(struct hdcdrv_service *service)
{
    mutex_init(&service->mutex);
    init_waitqueue_head(&service->wq_conn_avail);
    INIT_LIST_HEAD(&service->serv_list);

    service->listen_pid = HDCDRV_INVALID;
    service->conn_list_head = NULL;
}

STATIC int hdcdrv_service_res_init(struct hdcdrv_service *service, int server_type)
{
    struct hdcdrv_serv_list_node *node = NULL;
    int i = 0;

    if (hdc_ctrl->service_attr[server_type].service_scope == HDCDRV_SERVICE_SCOPE_GLOBAL) {
        return HDCDRV_OK;
    }

    if (list_empty_careful(&service->serv_list) != 0) {
        for (i = 0; i < HDCDRV_SERVER_PROCESS_MAX_NUM; i++) {
            node = (struct hdcdrv_serv_list_node *)kzalloc(sizeof(struct hdcdrv_serv_list_node),
                GFP_KERNEL | __GFP_ACCOUNT);
            if (unlikely(node == NULL)) {
                hdcdrv_err("Calling kzalloc failed. (process_num=%d; server_type=%d)\n", i, server_type);
                return HDCDRV_ERR;
            }

            hdcdrv_service_init(&node->service);
            list_add(&node->list, &service->serv_list);
        }
    }

    return HDCDRV_OK;
}

struct hdcdrv_dev *hdcdrv_add_dev(struct device *dev, u32 dev_id)
{
    struct hdcdrv_dev *hdc_dev = NULL;
    int i;

    if (dev_id >= (u32)hdcdrv_get_max_support_dev()) {
        hdcdrv_err("Input pararmeter is error. (dev_id=%u; max=%d)\n", dev_id, hdcdrv_get_max_support_dev());
        return NULL;
    }

    hdc_dev = &hdc_ctrl->devices[dev_id];

    if (hdc_dev->sync_mem_buf == NULL) {
        hdc_dev->sync_mem_buf = (void *)vzalloc(HDCDRV_NON_TRANS_MSG_S_DESC_SIZE);
        if (hdc_dev->sync_mem_buf == NULL) {
            hdcdrv_err("Calling alloc failed. (dev_id=%d)\n", dev_id);
            return NULL;
        }
    }
    /* suspend&resume: server not free */
    if (hdcdrv_get_running_status() == HDCDRV_RUNNING_NORMAL) {
        mutex_lock(&hdc_dev->mutex);
        for (i = 0; i < HDCDRV_SUPPORT_MAX_SERVICE; i++) {
            if (hdcdrv_service_res_init(&hdc_dev->service[i], i) != 0) {
                mutex_unlock(&hdc_dev->mutex);
                hdcdrv_err("Server resure init failed. (dev_id=%d; server=%d)\n", dev_id, i);
                goto OUT;
            }
        }
        mutex_unlock(&hdc_dev->mutex);
    }
    hdc_dev->dev = dev;
    hdc_dev->dev_id = dev_id;

    return hdc_dev;
OUT:
    if (hdc_dev->sync_mem_buf != NULL) {
        vfree(hdc_dev->sync_mem_buf);
    }

    for (i = 0; i < HDCDRV_SUPPORT_MAX_SERVICE; i++) {
        hdcdrv_service_res_uninit(&hdc_dev->service[i], i);
    }

    return NULL;
}

STATIC void hdcdrv_reset_session(int dev_id)
{
    struct hdcdrv_session *session = NULL;
    int id;

    /* only uninit free, suspend status session not free */
    if (hdcdrv_get_running_status() != HDCDRV_RUNNING_NORMAL) {
        return;
    }
    for (id = 0; id < HDCDRV_REAL_MAX_SESSION; id++) {
        session = &hdc_ctrl->sessions[id];
        if (session->dev_id != dev_id) {
            continue;
        }
        if (hdcdrv_get_session_status(session) == HDCDRV_SESSION_STATUS_IDLE) {
            if (delayed_work_pending(&session->remote_close) != 0) {
                (void)cancel_delayed_work_sync(&session->remote_close);
            }
            continue;
        }
        hdcdrv_info("Get reset info. (dev=%d; session=%d)\n", dev_id, id);
        hdcdrv_session_free(session);
    }
}

STATIC void hdcdrv_reset_process_server(struct hdcdrv_dev *dev, int service_type)
{
    struct hdcdrv_serv_list_node *node = NULL;
    struct list_head *pos = NULL, *n = NULL;
    struct hdcdrv_service *service = NULL;

    service = &dev->service[service_type];

    if (hdc_ctrl->service_attr[service_type].service_scope == HDCDRV_SERVICE_SCOPE_GLOBAL) {
        return;
    }

    mutex_lock(&dev->mutex);
    if (!list_empty_careful(&service->serv_list)) {
        list_for_each_safe(pos, n, &service->serv_list) {
            node = list_entry(pos, struct hdcdrv_serv_list_node, list);
            if (node->service.listen_status == HDCDRV_VALID) {
                (void)hdcdrv_server_free(&node->service, (int)dev->dev_id, service_type);
            }
        }
    }
    mutex_unlock(&dev->mutex);

    return;
}

STATIC void hdcdrv_reset_service(struct hdcdrv_dev *hdc_dev)
{
    struct hdcdrv_service *service = NULL;
    int i;
    int ret = 0;

    /* only uninit free, suspend status server not free */
    if (hdcdrv_get_running_status() != HDCDRV_RUNNING_NORMAL) {
        return;
    }

    for (i = 0; i < HDCDRV_SUPPORT_MAX_SERVICE; i++) {
        service = &hdc_dev->service[i];
        if (service->listen_status == HDCDRV_VALID) {
            hdcdrv_info("Service wakeup accept. (dev=%d; service=%d)\n", hdc_dev->dev_id, i);
            ret = (int)hdcdrv_server_free(service, (int)hdc_dev->dev_id, i);
            if (ret != HDCDRV_OK) {
                hdcdrv_warn("Reset not success. (dev=%d; service=%d)\n", hdc_dev->dev_id, i);
            }
        }

        hdcdrv_reset_process_server(hdc_dev, i);
    }
}

STATIC void hdcdrv_reset_msgchain(struct hdcdrv_dev *hdc_dev)
{
    struct hdcdrv_msg_chan *msg_chan = NULL;
    int i;

    for (i = 0; i < hdc_dev->msg_chan_cnt; i++) {
        msg_chan = hdc_dev->msg_chan[i];
        /* Clear the packets in the waiting queue */
        wake_up_interruptible(&msg_chan->send_wait);
    }
}

STATIC void hdcdrv_kill_task(struct hdcdrv_dev *hdc_dev)
{
    struct hdcdrv_msg_chan *msg_chan = NULL;
    int msg_chan_num = hdc_dev->msg_chan_cnt;
    int i;

    for (i = 0; i < msg_chan_num; i++) {
        msg_chan = hdc_dev->msg_chan[i];
        (void)hdcdrv_set_msg_chan_priv(msg_chan->chan, NULL);
        tasklet_kill(&msg_chan->tx_finish_task);
        if ((g_rx_notify_sched_mode == HDCDRV_RX_SCHED_WORK) ||
            (devdrv_is_mdev_vm_boot_mode(msg_chan->dev_id) == true)) {
            destroy_workqueue(msg_chan->rx_workqueue);
        } else {
            tasklet_kill(&msg_chan->rx_notify_task);
        }
        tasklet_kill(&msg_chan->rx_task);
    }
}

void hdcdrv_free_msg_chan(struct hdcdrv_dev *hdc_dev)
{
    int msg_chan_num = hdc_dev->msg_chan_cnt;
    int i;

    for (i = 0; i < msg_chan_num; i++) {
        mutex_lock(&hdc_dev->mutex);
        hdc_dev->msg_chan_cnt--;
        mutex_unlock(&hdc_dev->mutex);
    }
}
STATIC void hdcdrv_free_msg_chan_mem_handle(struct hdcdrv_buf_desc *desc)
{
    if (desc->buf != NULL) {
        if (desc->fid == HDCDRV_DEFAULT_PM_FID) {
#ifdef CFG_FEATURE_MIRROR
            hdcdrv_free_mem_mirror(desc->buf, desc->mem_id, desc->len);
#else
            free_mem(desc->buf);
#endif
        }
        desc->buf = NULL;
    }
}
STATIC void hdcdrv_free_msg_chan_mem(struct hdcdrv_dev *hdc_dev)
{
    struct hdcdrv_msg_chan *msg_chan = NULL;
    int msg_chan_num = hdc_dev->msg_chan_cnt;
    int i, j;

    for (i = 0; i < msg_chan_num; i++) {
        msg_chan = hdc_dev->msg_chan[i];
        for (j = 0; j < HDCDRV_DESC_QUEUE_DEPTH; j++) {
            hdcdrv_free_msg_chan_mem_handle(&msg_chan->tx[j]);
            hdcdrv_free_msg_chan_mem_handle(&msg_chan->rx[j]);
        }
    }
}

void hdcdrv_stop_work(struct hdcdrv_dev *hdc_dev)
{
    int dev_id = (int)hdc_dev->dev_id;

    hdcdrv_reset_session(dev_id);
    hdcdrv_reset_service(hdc_dev);
    hdcdrv_kill_task(hdc_dev);
}

void hdcdrv_remove_dev(struct hdcdrv_dev *hdc_dev)
{
    int dev_id = (int)hdc_dev->dev_id;
    hdcdrv_free_msg_chan_mem(hdc_dev);
    hdcdrv_free_msg_chan(hdc_dev);
    hdcdrv_uninit_mem_pool((u32)dev_id);
    hdcdrv_fast_mem_uninit(&(hdc_ctrl->devices[dev_id].fmem.rb_lock),
        &(hdc_ctrl->devices[dev_id].fmem.rbtree_re), HDCDRV_TRUE_FLAG, HDCDRV_DEL_FLAG);
    hdcdrv_fast_mem_sep_uninit(&(hdc_ctrl->devices[dev_id].fmem.rb_lock),
        &(hdc_ctrl->devices[dev_id].fmem.rbtree));
}

struct hdcdrv_dev *hdcdrv_get_dev(u32 dev_id)
{
    return &hdc_ctrl->devices[dev_id];
}

STATIC void hdcdrv_free_dev_mem(u32 dev_id)
{
    struct hdcdrv_dev *hdc_dev = &hdc_ctrl->devices[dev_id];
    int i;

    for (i = 0; i < HDCDRV_SUPPORT_MAX_DEV_MSG_CHAN; i++) {
        if (hdc_dev->msg_chan[i] != NULL) {
            if (hdc_dev->msg_chan[i]->node != NULL) {
                kfree(hdc_dev->msg_chan[i]->node);
                hdc_dev->msg_chan[i]->node = NULL;
            }
            hdcdrv_kvfree(hdc_dev->msg_chan[i]);
            hdc_dev->msg_chan[i] = NULL;
        }
    }

    if (hdc_dev->sync_mem_buf != NULL) {
        vfree(hdc_dev->sync_mem_buf);
        hdc_dev->sync_mem_buf = NULL;
    }
}

void hdcdrv_del_dev(u32 dev_id)
{
    struct hdcdrv_dev *hdc_dev = &hdc_ctrl->devices[dev_id];

    if (hdc_dev->valid == HDCDRV_INVALID) {
        return;
    }

    hdcdrv_reset_dev(hdc_dev);
    msleep(500);
    hdcdrv_stop_work(hdc_dev);
    hdcdrv_remove_dev(hdc_dev);
}

void hdcdrv_reset_dev(struct hdcdrv_dev *hdc_dev)
{
    hdcdrv_info("Reset device. (dev_id=%u)\n", hdc_dev->dev_id);

    hdcdrv_set_device_status((int)hdc_dev->dev_id, HDCDRV_INVALID);
    msleep(10);
    hdcdrv_reset_msgchain(hdc_dev);
    hdcdrv_reset_session((int)hdc_dev->dev_id);
    hdcdrv_reset_service(hdc_dev);
}

const struct file_operations hdcdrv_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = hdcdrv_ioctl,
    .open = hdcdrv_open,
    .mmap = hdcdrv_mmap,
    .release = hdcdrv_release,
};

STATIC int hdcdrv_register_cdev(void)
{
    int ret;

    ret = hdccom_register_cdev(&hdc_ctrl->hdc_cdev, &hdcdrv_fops);
    if (ret != HDCDRV_OK) {
        hdcdrv_err("HDC create char device failed. (ret=%d)\n", ret);
        return ret;
    }

    return HDCDRV_OK;
}

STATIC void hdcdrv_free_cdev(void)
{
    hdccom_free_cdev(&hdc_ctrl->hdc_cdev);
}

int hdcdrv_get_running_status(void)
{
#ifdef CFG_FEATURE_HDC_REG_MEM
    return hdc_ctrl->running_statue;
#else
    return 0;
#endif
}

void hdcdrv_set_running_status(int statue)
{
#ifdef CFG_FEATURE_HDC_REG_MEM
    hdc_ctrl->running_statue = statue;
#else
    return;
#endif
}

#ifdef CFG_FEATURE_HDC_REG_MEM
STATIC int hdcdrv_session_free_check(void)
{
    int id = 0;
    struct hdcdrv_session *session = NULL;

    /* check session status: before suspend, all session must be closed */
    for (id = 0; id < HDCDRV_REAL_MAX_SESSION; id++) {
        session = &hdc_ctrl->sessions[id];
        if (session == NULL) {
            hdcdrv_err("hdcdrv_suspend fail, session(id=%d) is null.\n", id);
            return HDCDRV_ERR;
        }
        if (hdcdrv_get_session_status(session) != HDCDRV_SESSION_STATUS_IDLE) {
            hdcdrv_err("hdcdrv_suspend fail, have session(id=%d, owner_pid=%llu) not release.\n",
                session->local_session_fd, session->owner_pid);
            return HDCDRV_ERR;
        }
    }
    return HDCDRV_OK;
}
#endif

int hdcdrv_suspend(u32 dev_id)
{
#ifdef CFG_FEATURE_HDC_REG_MEM
    int ret;
    u32 dev_idx;

    hdcdrv_set_running_status(HDCDRV_RUNNING_SUSPEND);
    hdcdrv_info("hdcdrv suspend start\n");

    /* check session status: before suspend, all session must be closed */
    ret = hdcdrv_session_free_check();
    if (ret != HDCDRV_OK) {
        hdcdrv_err("hdcdrv_suspend fail.\n");
        return HDCDRV_ERR;
    }

    (void)cancel_delayed_work_sync(&hdc_ctrl->recycle);
    (void)cancel_delayed_work_sync(&hdc_ctrl->recycle_mem);

    ret = hdcdrv_uninit_instance(dev_id);
    if (ret != HDCDRV_OK) {
        hdcdrv_err("hdcdrv_suspend fail.\n");
        return HDCDRV_ERR;
    }

    for (dev_idx = 0; dev_idx < (u32)hdcdrv_get_max_support_dev(); dev_idx++) {
        hdcdrv_free_dev_mem(dev_idx);
    }
#endif
    hdcdrv_info("hdcdrv suspend success\n");
    return 0;
}

int hdcdrv_resume(u32 dev_id, struct device *dev)
{
#ifdef CFG_FEATURE_HDC_REG_MEM
    int ret;

    hdcdrv_info("hdcdrv resume start\n");
    hdcdrv_set_running_status(HDCDRV_RUNNING_RESUME);
    INIT_DELAYED_WORK(&hdc_ctrl->recycle, hdcdrv_guard_work);
    INIT_DELAYED_WORK(&hdc_ctrl->recycle_mem, hdcdrv_recycle_mem_work);
    (void)schedule_delayed_work(&hdc_ctrl->recycle, HDCDRV_RECYCLE_DELAY_TIME * HZ);

    ret = hdcdrv_init_instance(dev_id, dev);
    if (ret != HDCDRV_OK) {
        hdcdrv_err("hdcdrv_suspend fail.\n");
        return HDCDRV_ERR;
    }
#endif
    hdcdrv_info("hdcdrv resume success\n");
    return 0;
}

STATIC void hdcdrv_service_attr_init(void)
{
    int i = 0;

    for (i = 0; i < HDCDRV_SUPPORT_MAX_SERVICE; i++) {
        hdc_ctrl->service_attr[i].level = hdcdrv_service_level_init(i);
        hdc_ctrl->service_attr[i].conn_feature = hdcdrv_service_conn_feature_init(i);
        hdc_ctrl->service_attr[i].service_scope = hdcdrv_service_scope_init(i);
    }
}

STATIC void hdcdrv_session_pre_init(struct hdcdrv_session *session)
{
#ifdef CFG_FEATURE_HDC_REG_MEM
    int ret = 0;
#endif
    spin_lock_init(&session->lock);
    init_waitqueue_head(&session->wq_conn);
    init_waitqueue_head(&session->wq_rx);
#ifdef CFG_FEATURE_HDC_REG_MEM
    ret = hdcdrv_init_session_release_mem_event(session);
    if (ret != HDCDRV_OK) {
        hdcdrv_set_session_status(session, HDCDRV_SESSION_STATUS_IDLE);
        hdcdrv_err("Init send mem finish event fifo failed.\n");
        return;
    }
#endif
    INIT_DELAYED_WORK(&session->remote_close, hdcdrv_remote_close_work);
    INIT_DELAYED_WORK(&session->close_unknow_session, hdcdrv_close_unknow_session_work);
#ifdef CFG_FEATURE_VFIO
    hdcdrv_session_work_init(&session->session_work);
#endif
    hdcdrv_set_session_status(session, HDCDRV_SESSION_STATUS_IDLE);
    session->owner = (u32)HDCDRV_INVALID_VALUE;
    session->owner_pid = HDCDRV_INVALID_PID;
    session->create_pid = HDCDRV_INVALID_PID;
    session->peer_create_pid = HDCDRV_INVALID_PID;
    session->local_close_state = HDCDRV_CLOSE_TYPE_NONE;
    session->remote_close_state = HDCDRV_CLOSE_TYPE_NONE;
    session->pid_flag = 0;
}
#ifdef CFG_FEATURE_HDC_REG_MEM
/* alloc node buff and init params */
STATIC struct hdcdrv_node_tree_ctrl *hdcdrv_ctrl_arry_init(void)
{
    int i;
    int j;
    struct hdcdrv_node_tree *node_tree = NULL;
    hdc_node_tree = (struct hdcdrv_node_tree_ctrl *)vzalloc(sizeof(struct hdcdrv_node_tree_ctrl));
    if (hdc_node_tree == NULL) {
        hdcdrv_err("hdc_node_tree calling alloc failed. (size=%ld)\n", sizeof(struct hdcdrv_node_tree_ctrl));
        return NULL;
    }

    mutex_init(&hdc_node_tree->mutex);
    for (i = 0; i < HDCDRV_SUPPORT_MAX_FID_PID; i++) {
        node_tree = &hdc_node_tree->node_tree[i];
        node_tree->local_tree.tree_info.pid = HDCDRV_INVALID_PID;
        node_tree->local_tree.tree_info.fid = HDCDRV_INVALID_FID;
        node_tree->local_tree.fmem.rbtree = RB_ROOT;
        node_tree->local_tree.fmem.rbtree_re = RB_ROOT;
        spin_lock_init(&node_tree->local_tree.fmem.rb_lock);
        spin_lock_init(&node_tree->local_tree.fmem.mem_dfx_stat.lock);

        node_tree->re_tree.tree_info.pid = HDCDRV_INVALID_PID;
        node_tree->re_tree.tree_info.fid = HDCDRV_INVALID_FID;
        for (j = 0; j < hdcdrv_get_max_support_dev(); j++) {
            node_tree->re_tree.devices[j].rbtree = RB_ROOT;
            node_tree->re_tree.devices[j].rbtree_re = RB_ROOT;
            spin_lock_init(&node_tree->re_tree.devices[j].rb_lock);
            spin_lock_init(&node_tree->re_tree.devices[j].mem_dfx_stat.lock);
        }
    }
    hdcdrv_info("hdc_node_tree init success.\n");
    return hdc_node_tree;
}
#endif

STATIC struct hdcdrv_ctrl *hdcdrv_ctrl_init(void)
{
    int i, j;
    int dev_num;

    dev_num = devdrv_get_davinci_dev_num();
    if (dev_num <= 0) {
        hdcdrv_info("No pci dev, set dev num %d to 1\n", dev_num);
        dev_num = 1;
    }
    if (dev_num > HDCDRV_SUPPORT_MAX_DEV) {
        hdcdrv_info("change dev num %d to max dev num %d\n", dev_num, HDCDRV_SUPPORT_MAX_DEV);
        dev_num = HDCDRV_SUPPORT_MAX_DEV;
    }

    hdcdrv_dev_num = (u32)dev_num;

    hdc_ctrl = (struct hdcdrv_ctrl *)vzalloc(sizeof(struct hdcdrv_ctrl));
    if (hdc_ctrl == NULL) {
        hdcdrv_err("Calling alloc failed. (size=%ld)\n", sizeof(struct hdcdrv_ctrl));
        return NULL;
    }

    hdc_ctrl->sessions = (struct hdcdrv_session *)vzalloc(sizeof(struct hdcdrv_session) * dev_num *
        HDCDRV_SINGLE_DEV_MAX_SESSION);
    if (hdc_ctrl->sessions == NULL) {
        vfree(hdc_ctrl);
        hdc_ctrl = NULL;
        hdcdrv_err("alloc sessions failed, size %ld dev_num %d single session %d\n", sizeof(struct hdcdrv_session),
            dev_num, HDCDRV_SINGLE_DEV_MAX_SESSION);
        return NULL;
    }

    hdcdrv_info("Sessions size %lu, dev_num %d, session num %d\n", sizeof(struct hdcdrv_session), dev_num,
        HDCDRV_REAL_MAX_SESSION);

    mutex_init(&hdc_ctrl->mutex);
    hdc_ctrl->running_statue = HDCDRV_RUNNING_NORMAL;
    hdc_ctrl->segment = HDCDRV_HUGE_PACKET_SEGMENT;
    hdc_ctrl->cur_alloc_short_session = (int)HDCDRV_REAL_MAX_LONG_SESSION;

    hdc_ctrl->fmem.rbtree = RB_ROOT;
    hdc_ctrl->fmem.rbtree_re = RB_ROOT;
    spin_lock_init(&hdc_ctrl->fmem.rb_lock);
    spin_lock_init(&hdc_ctrl->fmem.mem_dfx_stat.lock);

    for (i = 0; i < hdcdrv_get_max_support_dev(); i++) {
        hdcdrv_set_peer_dev_id(i, HDCDRV_INVALID_VALUE);
        mutex_init(&hdc_ctrl->devices[i].sync_mem_mutex);
        mutex_init(&hdc_ctrl->devices[i].mutex);

        hdc_ctrl->devices[i].fmem.rbtree = RB_ROOT;
        hdc_ctrl->devices[i].fmem.rbtree_re = RB_ROOT;
        spin_lock_init(&hdc_ctrl->devices[i].fmem.rb_lock);
        spin_lock_init(&hdc_ctrl->devices[i].fmem.mem_dfx_stat.lock);

        for (j = 0; j < HDCDRV_SUPPORT_MAX_SERVICE; j++) {
            hdcdrv_service_init(&hdc_ctrl->devices[i].service[j]);
        }
    }
    hdc_ctrl->debug_state.valid = HDCDRV_INVALID;
    hdc_ctrl->debug_state.pid = HDCDRV_INVALID_VALUE;

    hdcdrv_service_attr_init();

    for (i = 0; i < hdcdrv_get_max_support_dev(); i++) {
        hdc_ctrl->devices[i].valid = HDCDRV_INVALID;
    }

    for (i = 0; i < HDCDRV_REAL_MAX_SESSION; i++) {
        hdcdrv_session_pre_init(&hdc_ctrl->sessions[i]);
    }

    hdc_ctrl->pm_version = (int)HDC_VERSION;
    hdcdrv_info("HDC PM_VERSION = %d.\n", hdc_ctrl->pm_version);

    return hdc_ctrl;
}

STATIC void hdcdrv_session_notify_init(void)
{
    int i = 0;

    for (i = 0; i < HDCDRV_SUPPORT_MAX_SERVICE; i++) {
        g_session_notify[i].connect_notify = NULL;
        g_session_notify[i].close_notify = NULL;
        g_session_notify[i].data_in_notify = NULL;
    }
}

int hdcdrv_init(void)
{
    int ret;

#if defined(CFG_FEATURE_VFIO) || defined(CFG_FEATURE_MIRROR)
    g_rx_notify_sched_mode = HDCDRV_RX_SCHED_WORK;
#endif

    hdcdrv_fill_service_type_str();

    hdc_ctrl = hdcdrv_ctrl_init();
    if (hdc_ctrl == NULL) {
        hdcdrv_err("Calling hdcdrv_ctrl_init failed. (size=%ld)\n", sizeof(struct hdcdrv_ctrl));
        goto HDC_CTRL_INIT_FAIL;
    }
#ifdef CFG_FEATURE_HDC_REG_MEM
    hdc_node_tree = hdcdrv_ctrl_arry_init();
    if (hdc_node_tree == NULL) {
        hdcdrv_err("Calling hdcdrv_ctrl_arry_init failed. (size=%ld)\n", sizeof(struct hdcdrv_node_tree_ctrl));
        goto HDC_CTRL_ARRY_INIT_FAIL;
    }
    hdcdrv_link_status_init();
#endif
    if (hdcdrv_register_cdev() != 0) {
        hdcdrv_err("Calling hdcdrv_register_cdev failed.\n");
        goto REGISTER_CDEV_FAIL;
    }

    hdcdrv_session_notify_init();

    ret = hdcdrv_epoll_init(&hdc_ctrl->epolls);
    if (ret != 0) {
        hdcdrv_err("Calling hdcdrv_epoll_init failed.\n");
        goto HDC_EPOLL_INIT_FAIL;
    }

    hdcdrv_sysfs_init(hdc_ctrl->hdc_cdev.dev);

    INIT_DELAYED_WORK(&hdc_ctrl->recycle, hdcdrv_guard_work);
    INIT_DELAYED_WORK(&hdc_ctrl->recycle_mem, hdcdrv_recycle_mem_work);
    (void)schedule_delayed_work(&hdc_ctrl->recycle, HDCDRV_RECYCLE_DELAY_TIME * HZ);
    rwlock_init(&g_symbol_lock);

    hdcdrv_info("Get sq cq desc size.(sq_size=%d; cq_size=%d)\n", (int)HDCDRV_SQ_DESC_SIZE, (int)HDCDRV_CQ_DESC_SIZE);
    return HDCDRV_OK;

HDC_EPOLL_INIT_FAIL:
    hdcdrv_free_cdev();
REGISTER_CDEV_FAIL:
#ifdef CFG_FEATURE_HDC_REG_MEM
    vfree(hdc_node_tree);
    hdc_node_tree = NULL;
HDC_CTRL_ARRY_INIT_FAIL:
#endif
    vfree(hdc_ctrl->sessions);
    hdc_ctrl->sessions = NULL;
    vfree(hdc_ctrl);
    hdc_ctrl = NULL;

HDC_CTRL_INIT_FAIL:
    return HDCDRV_MEM_ALLOC_FAIL;
}

void hdcdrv_uninit(void)
{
    u32 dev_id;
    int server_type;
#ifdef CFG_FEATURE_HDC_REG_MEM
    int id = 0;
    struct hdcdrv_session *session = NULL;
    unsigned long flags;
#endif

    (void)cancel_delayed_work_sync(&hdc_ctrl->recycle);
    (void)cancel_delayed_work_sync(&hdc_ctrl->recycle_mem);

    hdcdrv_sysfs_uninit(hdc_ctrl->hdc_cdev.dev);
    hdcdrv_free_cdev();
#ifdef CFG_FEATURE_HDC_REG_MEM
    hdcdrv_fast_mem_arry_uninit();
    hdcdrv_link_status_uinit();
#endif

    for (dev_id = 0; dev_id < (u32)hdcdrv_get_max_support_dev(); dev_id++) {
        hdcdrv_del_dev(dev_id);
        hdcdrv_free_dev_mem(dev_id);
        hdcdrv_fast_mem_sep_uninit(&(hdc_ctrl->devices[dev_id].fmem.rb_lock),
            &(hdc_ctrl->devices[dev_id].fmem.rbtree));
        for (server_type = 0; server_type < HDCDRV_SUPPORT_MAX_SERVICE; server_type++) {
            hdcdrv_service_res_uninit(&hdc_ctrl->devices[dev_id].service[server_type], server_type);
        }
    }

    hdcdrv_fast_mem_uninit(&(hdc_ctrl->fmem.rb_lock), &(hdc_ctrl->fmem.rbtree), HDCDRV_TRUE_FLAG, HDCDRV_DEL_FLAG);

    hdcdrv_epoll_uninit(&hdc_ctrl->epolls);

    // free mem release fifo buffer when hdc driver uninit
#ifdef CFG_FEATURE_HDC_REG_MEM
    for (id = 0; id < HDCDRV_REAL_MAX_SESSION; id++) {
        session = &hdc_ctrl->sessions[id];
        if (session->mem_release_fifo.buf != NULL) {
            spin_lock_irqsave(&session->mem_release_lock, flags);
            kfifo_free(&session->mem_release_fifo);
            spin_unlock_irqrestore(&session->mem_release_lock, flags);
        }
    }

    vfree(hdc_node_tree);
    hdc_node_tree = NULL;
#endif

#if defined(CFG_FEATURE_VFIO) || defined(CFG_FEATURE_MIRROR)
        g_rx_notify_sched_mode = HDCDRV_RX_SCHED_TASKLET;
#endif

    vfree(hdc_ctrl->sessions);
    hdc_ctrl->sessions = NULL;
    vfree(hdc_ctrl);
    hdc_ctrl = NULL;
}
