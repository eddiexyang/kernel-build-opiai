/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: gfbg hal operation
 */

#ifndef TDE_OSR_H
#define TDE_OSR_H

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

td_void tde_osr_enableirq(td_void);
td_void tde_osr_disableirq(td_void);
td_void tde_osr_hsr(td_void* pstFunc, td_void* data);
struct proc_dir_entry *tde_get_procentry(td_void);

#endif /* TDE_OSR_H */
