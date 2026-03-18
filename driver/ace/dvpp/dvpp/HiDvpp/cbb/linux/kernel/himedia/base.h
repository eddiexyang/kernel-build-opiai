/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2008-2021. All rights reserved.
* Description:base
* Author: Hisilicon multimedia software group
* Create: 2008/02/21
*/

#ifndef BASE_H_
#define BASE_H_

#include "himedia.h"

hi_s32 himedia_bus_init(void);
void himedia_bus_exit(void);

hi_s32 himedia_device_register(struct himedia_device *pdev);

void himedia_device_unregister(struct himedia_device *pdev);

struct himedia_driver *himedia_driver_register(const hi_char *name,
                                               struct module *owner, struct himedia_ops *ops);

void himedia_driver_unregister(struct himedia_driver *pdrv);

#endif // BASE_H_
