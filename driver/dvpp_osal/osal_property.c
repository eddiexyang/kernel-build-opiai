/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description:
 * Author: huawei
 * Create: 2023-4-1
 */
#include "hi_osal.h"

#include <linux/property.h>
#include <linux/of.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif

hi_s32 osal_device_property_read_string(void *dev, const char *propname, const char **val)
{
    return device_property_read_string((struct device*)dev, propname, val);
}
EXPORT_SYMBOL(osal_device_property_read_string);

hi_s32 osal_device_property_read_u32(void *dev, const char *propname, hi_u32 *val)
{
    return device_property_read_u32_array((struct device*)dev, propname, val, 1);
}
EXPORT_SYMBOL(osal_device_property_read_u32);

hi_s32 osal_device_property_read_u32_array(void *dev, const char *propname, hi_u32 *val, size_t nval)
{
    return device_property_read_u32_array((struct device*)dev, propname, val, nval);
}
EXPORT_SYMBOL(osal_device_property_read_u32_array);

hi_s32 osal_device_property_read_u64(void *dev, const char *propname, u64 *val)
{
    return device_property_read_u64_array((struct device*)dev, propname, val, 1);
}
EXPORT_SYMBOL(osal_device_property_read_u64);

hi_s32 osal_device_property_read_u64_array(void *dev, const char *propname, u64 *val, size_t nval)
{
    return device_property_read_u64_array((struct device*)dev, propname, val, nval);
}
EXPORT_SYMBOL(osal_device_property_read_u64_array);

hi_s32 osal_of_property_read_u32_index(const void *np,
    const char *propname, hi_u32 idx, hi_u32 *out_value)
{
    return of_property_read_u32_index((const struct device_node*)np, propname, idx, out_value);
}
EXPORT_SYMBOL(osal_of_property_read_u32_index);

hi_s32 osal_of_property_count_u32_elems(const void *np, const char *propname)
{
    return of_property_count_u32_elems((const struct device_node*)np, propname);
}
EXPORT_SYMBOL(osal_of_property_count_u32_elems);

hi_s32 osal_of_property_read_u64_array(const void *np, const char *propname, u64 *out_values, size_t sz)
{
    return of_property_read_u64_array((const struct device_node*)np, propname, out_values, sz);
}
EXPORT_SYMBOL(osal_of_property_read_u64_array);

hi_s32 osal_of_property_read_u32_array(const void *np,
    const char *propname, hi_u32 *out_values, size_t sz)
{
    return of_property_read_u32_array((const struct device_node*)np, propname, out_values, sz);
}
EXPORT_SYMBOL(osal_of_property_read_u32_array);

hi_s32 osal_of_property_read_u32(const void *np, const char *propname, hi_u32 *out_value)
{
    return of_property_read_u32((const struct device_node*)np, propname, out_value);
}
EXPORT_SYMBOL(osal_of_property_read_u32);
