/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: pid_protect.h
 * Author: Hisilicon multimedia software group
 * Create: 2023-03-05
 */

#ifndef PID_PROTECT
#define PID_PROTECT

#include <linux/clk.h>
#include "hi_comm_audio.h"
#include "ot_inner_common_aio.h"

td_bool check_is_same_pid(td_bool dev_open);
td_void check_is_close(void);
td_void check_pid_spin_lock_init(void);
td_void check_pid_spin_lock_destroy(void);
#define MAX_OPEN_CNT 160
td_s32 audio_config_smmu(void);
#endif

