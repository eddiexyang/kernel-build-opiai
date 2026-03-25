/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: define for sysmonitor event msg and function
 * Author: tangyizhou
 * Create: 2019-3-20
 */
#ifndef SYSMONITOR_H
#define SYSMONITOR_H

enum sysmonitor_event_type {
        SIGNAL
};

unsigned long get_sigcatchmask(void);
int save_msg(int type, const void *msg, int msg_size);
#endif
