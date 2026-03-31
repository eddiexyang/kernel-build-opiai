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

#ifndef _DRV_PROFILE_H_
#define _DRV_PROFILE_H_

#define SAMPLE_MASK 0x01
#define SAMPLE_ONLY_DATA 0x0
#define SAMPLE_WITH_HEADER 0x1

#define SAMPLE_NAME_MAX 16

#define PROF_PERI_REV_NUM 6
struct prof_peri_para {
    unsigned int device_id;
    unsigned int vfid;
    void *user_data;                /* sample 配置信息 */
    unsigned int user_data_len;     /* sample 配置信息数据长度 */
    unsigned int sample_flag;       /* 标识是否为首次 sample, SAMPLE_ONLY_DATA：非首次；SAMPLE_WITH_HEADER：首次 */
    void *buff;                     /* sample buff地址 */
    unsigned int buff_len;          /* sample buff总长度 */
    int target_pid;
    unsigned int reserve[PROF_PERI_REV_NUM];
};

struct prof_sample_start_para {
    unsigned int dev_id;
    void *user_data;                /* sample 配置信息 */
    unsigned int user_data_len;     /* sample 配置信息数据长度 */
};

struct prof_sample_para {
    unsigned int dev_id;
    unsigned int sample_flag;       /* 标识是否为首次 sample, SAMPLE_ONLY_DATA：非首次；SAMPLE_WITH_HEADER：首次 */
    void *buff;                     /* sample buff地址 */
    unsigned int buff_len;          /* sample buff总长度 */
    unsigned int report_len;        /* 数据量 */
};

struct prof_sample_stop_para {
    unsigned int dev_id;
};

struct prof_sample_ops {
    int (*start_fun)(struct prof_sample_start_para *para);  /* not must */
    int (*sample_fun)(struct prof_sample_para *para);
    int (*stop_fun)(struct prof_sample_stop_para *para);   /* not must */
};
/* prof_sample_register :   采样注册函数
 * struct prof_sample_ops *ops : 注册的回调函数
 * channel_id: 通道id编号
 */
void prof_sample_register(unsigned int dev_id, unsigned int chan_id, struct prof_sample_ops *ops);
#endif /* _DRV_PROFILE_H_ */

