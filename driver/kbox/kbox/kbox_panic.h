/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox record panic scene log
 * Author: wangshouping
 * Create: 2019-02-14
 */

#ifndef __KBOX_PANIC_H
#define __KBOX_PANIC_H

#define PANIC_REASON_UNKOW "unknown panic reason!"
#define PANIC_RECORDED (-1)

#define PANIC_ON_HARDDOG "Hard LOCKUP"
#define PANIC_ON_HARDDOG_LEN (strlen(PANIC_ON_HARDDOG))

#define PANIC_ON_SOFTDOG "softlockup: hung tasks"
#define PANIC_ON_SOFTDOG_LEN (strlen(PANIC_ON_SOFTDOG))

#define PANIC_ON_OOM "Out of memory"
#define PANIC_ON_OOM_LEN (strlen(PANIC_ON_OOM))

#define PANIC_ON_DIE "Fatal exception"
#define PANIC_ON_DIE_LEN (strlen(PANIC_ON_DIE))

int kbox_panic_init(void);
void kbox_panic_fini(void);
#endif
