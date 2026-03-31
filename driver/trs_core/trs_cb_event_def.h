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
 * Create: 2022-8-15
 */

#ifndef TRS_CB_EVNET_DEF_H
#define TRS_CB_EVNET_DEF_H

#define TRS_CB_EVENT_GRP_ID 11
#define TRS_CB_GROUP_NUM 1024

#define TRS_CB_HW_SUBEVENTID 0x0 /* stars */
#define TRS_CB_SW_SUBEVENTID 0x1 /* trs drv send */
#define TRS_CB_HW_TIMEOUT_SUBEVENTID 0x0FEEU // 12bit for subtopic_id

struct trs_cb_cqe {
    unsigned short phase : 1;
    unsigned short SOP : 1;    /* start of packet, indicates this is the first 32bit return payload */
    unsigned short MOP : 1;    /* middle of packet, indicates the payload is a continuation of previous
        task return payload */
    unsigned short EOP : 1;    /* end of packet, indicates this is the last 32bit return payload.
                        SOP & EOP can appear in the same packet, MOP & EOP can also appear on the same packet. */
    unsigned short cq_id : 12; /* logic cq id */
    unsigned short stream_id;
    unsigned short task_id;
    unsigned short sq_id; /* physical sq id */
    unsigned short sq_head; /* physical sq head */
    unsigned short sequence_id; /* for match */
    unsigned char is_block;
    unsigned char reserved1;
    unsigned short event_id;
    unsigned long long host_func_cb_ptr;
    unsigned long long fn_data_ptr;
};

struct trs_cb_stars_event {
    unsigned int cqid : 16;
    unsigned int cb_groupid : 16;
    unsigned int devid : 16;
    unsigned int stream_id : 16;
    unsigned int event_id : 16;
    unsigned int is_block : 16;
    unsigned int task_id : 16;
    unsigned int res1 : 16;
    unsigned int host_func_low;
    unsigned int host_func_high;
    unsigned int fn_data_low;
    unsigned int fn_data_high;
};

static inline void trs_get_cb_group_num(unsigned int *group_num)
{
    *group_num = TRS_CB_GROUP_NUM;
}
#endif

