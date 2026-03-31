/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox proc
 * Author:
 * Create: 2019-02-14
 */
#ifndef _KBOX_KBOX_PROC_H_
#define _KBOX_KBOX_PROC_H_

#define KBOX_PROC_NAME "kbox"
#define KBOX_PROC_NAME_DEV "devs"
#define KBOX_PROC_NAME_MEMINFO "mem_info"

int kbox_proc_init(void);
void kbox_proc_fini(void);
#endif