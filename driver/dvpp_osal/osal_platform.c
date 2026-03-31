/*
* Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
*/

#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif // #ifdef AOS_LLVM_BUILD
hi_s32 osal_platform_driver_register(void *drv)
{
#if defined(AOS_LLVM_BUILD)
    return platform_driver_register((struct platform_driver *)drv);
#else
    return __platform_driver_register((struct platform_driver *)drv, THIS_MODULE);
#endif
}
EXPORT_SYMBOL(osal_platform_driver_register);

void osal_platform_driver_unregister(void *drv)
{
    platform_driver_unregister((struct platform_driver *)drv);
}
EXPORT_SYMBOL(osal_platform_driver_unregister);

void *osal_platform_device_register_simple(const char *name, hi_s32 id, const void *res, hi_u32 num)
{
    return platform_device_register_simple(name, id, (const struct resource *)res, num);
}
EXPORT_SYMBOL(osal_platform_device_register_simple);

int osal_platform_device_register(void *pdev)
{
    return platform_device_register((struct platform_device *)pdev);
}
EXPORT_SYMBOL(osal_platform_device_register);

void osal_platform_device_unregister(void *pdev)
{
    platform_device_unregister((struct platform_device *)pdev);
}
EXPORT_SYMBOL(osal_platform_device_unregister);

void *osal_platform_get_resource_byname(void *dev, hi_u32 type, const hi_char *name)
{
    return (void *)platform_get_resource_byname((struct platform_device *)dev, type, name);
}
EXPORT_SYMBOL(osal_platform_get_resource_byname);

void *osal_platform_get_resource(void *dev, hi_u32 type, hi_u32 num)
{
    return (void *)platform_get_resource((struct platform_device *)dev, type, num);
}
EXPORT_SYMBOL(osal_platform_get_resource);

hi_s32 osal_platform_get_irq(void *dev, hi_u32 num)
{
    return platform_get_irq((struct platform_device *)dev, num);
}
EXPORT_SYMBOL(osal_platform_get_irq);

hi_s32 osal_platform_get_irq_byname(void *dev, const hi_char *name)
{
    return platform_get_irq_byname((struct platform_device *)dev, name);
}
EXPORT_SYMBOL(osal_platform_get_irq_byname);

#ifdef AOS_LLVM_BUILD
hi_u32 osal_get_os_type(void)
{
    return OASL_OS_AOSCORE;
}
#else
hi_u32 osal_get_os_type(void)
{
    return OASL_OS_LINUX;
}
#endif
EXPORT_SYMBOL(osal_get_os_type);