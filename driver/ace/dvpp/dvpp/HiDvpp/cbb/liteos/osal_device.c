/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal device source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"
#include "los_spinlock.h"
#include "los_typedef.h"
#include "osal_ioctl.h"
#include "securec.h"

typedef struct osal_coat_dev {
    struct osal_dev osal_dev;
} osal_coat_dev_t;

void osal_device_init(void)
{
}

osal_dev_t *osal_createdev(const char *name)
{
    static osal_coat_dev_t dev = {};
    HI_UNUSED(name);
    return &(dev.osal_dev);
}

int osal_destroydev(const osal_dev_t *osal_dev)
{
    HI_UNUSED(osal_dev);
    return 0;
}

int osal_registerdevice(osal_dev_t *osal_dev)
{
    HI_TRACE_OSAL(HI_DBG_WARN, "not support in liteos\n");
    return 0;
}

void osal_deregisterdevice(const osal_dev_t *pdev)
{
    HI_TRACE_OSAL(HI_DBG_WARN, "not support in liteos\n");
}
