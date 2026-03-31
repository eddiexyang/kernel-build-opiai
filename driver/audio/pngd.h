/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
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
 * Create: 2020-4-1
 */

#ifndef __PNGD_H__
#define __PNGD_H__

#include "mkp_pngd.h"
#include "pngd_buf.h"
#include "pngd_drv.h"
#include "HiDvppPngdInternal.h"
#include "media_mem_file.h"
#include "dvpp_comm_ext.h"
#include "virtmng_interface.h"

#define MAX_PNGD_FRAME_NUM     40

#define THREAD_SLEEP_1_MS      1    /* 1ms  */
#define THREAD_SLEEP_10_MS     10   /* 10ms */

#define PNGD_STATE_STARTED     0
#define PNGD_STATE_STOPING     1
#define PNGD_STATE_STOPED      2

typedef struct {
    hi_pic_info png_pic_info;
    hi_img_stream stream;
} pngd_image_info;

typedef enum {
    PNGD_CREATED = 1,
    PNGD_CREATING,
    PNGD_RESETING,
    PNGD_DESTROYING,
    PNGD_DESTROYED
} pngd_operation;

typedef struct {
    // parameters of PNGD.
    hi_pngd_chn chn_id;
    hi_pngd_chn user_chn_id;
    hi_pngd_chn_attr attr;

    hi_u32 total_frames_in_buf;
    hi_u32 amount_stream; // 用于统计发送码率
    hi_u32 amount_send_frames; /* 用于统计发送帧率 */
    hi_u32 amount_frames; /* 用于统计解码帧率 */
    pngd_buf frame_info_buf;
    osal_semaphore_t pngd_sem;
    osal_spinlock_t stream_spin_lock;

    osal_wait_t wait_stream;
    osal_wait_t wait_pic;

    volatile hi_s32 stream_milli_sec;
    volatile hi_s32 unique_id; /* 标识通道是否被reset或重新创建 */
    volatile pngd_operation chn_state;
    hi_u32 total_perform; /* 已解码流性能统计数 */

    // acl 场景相关结构体
    hi_bool is_acl; // 0:直调场景 1：acl场景
    hi_bool is_stream_buf_full; // 0:stream buff未满 1：stream buff满
    hi_void *user_data; // acl场景需要回传给acl的指针
    hi_u32 pid;
    hi_bool aclmpi_mode; // 标记通道为host aclmpi模式
    osal_timer_t mpi_timer; // himpi模式，输入超时定时器
    hi_bool mpi_timer_pendding; // 超时定时器在运行

    struct media_mem_file mfile;
    smmu_resource smmu;
    hi_u32 vf_id;
    hi_u32 logic_dev_id;
    hi_u32 ext_phy_dev_id;
} pngd_context;

typedef struct {
    hi_u32 real_stream_rate;
    hi_u32 real_send_frame_rate;
    hi_u64 mpi_get_pics;

    hi_u64 mpi_send_strm_times;
    hi_u64 mpi_timer_setup;  // 输入超时定时器创建次数
    hi_u64 mpi_timer_kill;   // 定时器kill次数
    hi_u64 mpi_timer_run;    // 输入超时定时器运行次数
    hi_u64 mpi_timer_timeout; // 输入超时次数

    // acl相关
    hi_u32 acl_is_full;
    hi_u64 acl_full_count;
    hi_u64 acl_get_frame_times;
    hi_u64 acl_submit_not_full_times;
    hi_u64 acl_submit_complete_succ_times;
} pngd_proc;

typedef enum {
    THREAD_CMD_START, /* 启动指令：启动处于停止状态的线程 */
    THREAD_CMD_SLEEP, /* 休眠指令：让线程休眠等待 */
    THREAD_CMD_STOP, /* 销毁指令：使线程停止运行 */
    THREAD_CMD_DESTROYED, /* 线程已被销毁 */
    THREAD_CMD_BUTT
} thread_cmd;

typedef struct {
    hi_s32 chn_id;
    hi_s32 unique_id;
    hi_u32 stream_len;
} pngd_wait_sendstream;

typedef struct {
    hi_s32 chn_id;
    hi_s32 unique_id;
} pngd_wait_image;

typedef struct {
    hi_u32 max_chn_num[DEVICE_NUM][MAX_VF_ID_NUM]; // 索引为vfid,表示切分后某一个vf可创建的最大的通道数
    hi_u32 created_chn_num[DEVICE_NUM][MAX_VF_ID_NUM]; // 索引为vfid,表示切分后某一个vf已创建的通道数
    osal_spinlock_t pngd_vf_lock[DEVICE_NUM][MAX_VF_ID_NUM]; // 每个vf的锁,操作created_chn_num/max_chn_num时互斥
    osal_spinlock_t pngd_vf_manager_lock[DEVICE_NUM]; // 每个device的锁, 操作calculated_chn_num以及初始化时互斥
    hi_u32 calculated_chn_num[DEVICE_NUM]; // 记录已经分配的通道个数
} pngd_chn_vf_manager;

extern hi_u32 g_pngd_state;
extern hi_u32 g_device_num;
extern hi_u32 PngdMaxChnNum;
extern pngd_proc *g_pngd_proc;
extern pngd_context *g_pngd_chn;
extern osal_atomic_t g_pngd_user_ref;
extern osal_spinlock_t g_pngd_state_spin_lock;

hi_s32 pngd_create(hi_s32 chn_id, pngd_create_info *cteate_info);
hi_s32 pngd_destroy(hi_s32 chn_id);

hi_s32 pngd_send_stream(hi_s32 chn_id, hi_img_stream *stream, hi_pic_info png_pic_info,
                        hi_u32 pid, hi_s32 milli_sec);

hi_s32 pngd_get_image(hi_s32 chn_id, hi_pic_info *png_pic_info, hi_img_stream *stream, hi_s32 milli_sec);

hi_s32 pngd_proc_show(const struct osal_proc_dir_entry *s);
hi_s32 pngd_chn_vf_manager_init(hi_void);
hi_void pngd_chn_vf_manager_uninit(hi_void);
hi_s32 pngd_calc_init(struct vmngd_client_instance *instance);
hi_s32 pngd_calc_uninit(struct vmngd_client_instance *instance);

/*
* @brief : 设置ACL参数,ACL或Host场景使用
* @param [in] chn_id : pngd通道号
* @param [in] acl_info : acl参数信息
* @return : 0：成功获取; 其他：详见错误码
*/
hi_s32 pngd_set_acl(hi_s32 chn_id, pngd_acl_info *acl_info);

/*
* @brief : mpi场景，处理SendStream时输入buffer满
* @param [in] pngd_chn : pngd通道上下文
* @param [in] milli_sec : 用户SendStream传入的超时时间
* @return : 0：成功获取; 其他：详见错误码
*/
hi_s32 pngd_mpi_handle_input_full(pngd_context *pngd_chn, hi_s32 milli_sec);

/*
* @brief : 启动mpi输入超时定时器
* @param [in] pngd_chn : pngd通道上下文
* @param [in] milli_sec : 定时时间
* @return : 0：成功获取; 其他：详见错误码
*/
hi_s32 pngd_mpi_setup_timer(pngd_context *pngd_chn, hi_s32 milli_sec);

/*
* @brief : 关闭mpi输入超时定时器
* @param [in] pngd_chn : pngd通道上下文
* @return : void
*/
hi_void pngd_mpi_stop_timer(pngd_context *pngd_chn);

/*
* @brief : 销毁mpi输入超时定时器
* @param [in] pngd_chn : pngd通道上下文
* @return : void
*/
hi_void pngd_mpi_destroy_timer(pngd_context *pngd_chn);

hi_s32 pngd_check_module_func(hi_payload_type type);

hi_s32 trans_pngd_chan_id_to_device_id(hi_s32 chan_id);
hi_s32 trans_pngd_chan_id_to_device_chan_id(hi_s32 chan_id);
hi_s32 trans_pngd_chan_id_to_usr_chn_id(hi_s32 chan_id);

#endif // end of #ifndef  __PNGD_H__
