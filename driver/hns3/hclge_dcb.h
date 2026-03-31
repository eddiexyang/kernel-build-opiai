/* SPDX-License-Identifier: GPL-2.0+ */
// Copyright (c) 2016-2017 Hisilicon Limited.

#ifndef __HCLGE_DCB_H__
#define __HCLGE_DCB_H__

#ifndef HCLGE_PLATFORM_DEV
#include "hclge_main.h"
#else
#include "hclge_plf_main.h"
#endif

#ifdef CONFIG_HNS3_DCB
void hclge_dcb_ops_set(compat_hdev *hdev);
#else
static inline void hclge_dcb_ops_set(compat_hdev *hdev) {}
#endif

#endif /* __HCLGE_DCB_H__ */
