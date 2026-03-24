/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi osal.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#ifndef HDMI_OSAL_H
#define HDMI_OSAL_H

#include "ot_type.h"

typedef enum {
    OSAL_RDONLY,
    OSAL_WRONLY,
    OSAL_RDWR,
    OSAL_BUTT
} osal_file_flag;

#define is_str_equal(p1, p2) ((((p1) != TD_NULL) && ((p2) != TD_NULL) && (osal_strlen(p1) == osal_strlen(p2)) && \
    (strncmp(p1, p2, osal_strlen(p1)) == 0)))

td_s32 hdmi_osal_file_write(struct file *file, const td_char *buf, td_u32 len);

td_u32 hdmi_osal_get_time_in_ms(td_void);

td_u64 hdmi_osal_get_time_in_us(td_void);

#endif /* HDMI_OSAL_H */

