/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: process the kernel restart event
 * Author:
 * Create: 2019-03-12
 */

#ifndef __KBOX_EMERGE_H_
#define __KBOX_EMERGE_H_

int kbox_emerge_init(void);
void kbox_emerge_fini(void);
extern unsigned long g_emerge_recorded_flags;
#endif
