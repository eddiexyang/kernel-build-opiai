/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2023-4-1
 */
#include "hi_osal.h"

#include <linux/version.h>
#include <linux/iommu.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif
#include "securec.h"

#define SMMU_NAME_MAX_LEN 16U

// svm_get_pasid实现在OS中
#ifdef AOS_LLVM_BUILD
int svm_get_pasid(pid_t vpid, int dev_id);
#else
int svm_get_pasid(pid_t vpid, int dev_id __maybe_unused);
/*
 * svm_find_domain_by_name is optionally provided by the SVM module.
 * Cannot use __attribute__((weak)) because GCC generates GOT-based
 * relocations (R_AARCH64_LD64_GOT_LO12_NC) which the kernel module
 * loader does not support.  Use compat_lookup_name() at runtime instead.
 */
#endif

hi_s32 osal_iommu_attach_group(void *domain, void *group)
{
    return iommu_attach_group((struct iommu_domain*)domain, (struct iommu_group*)group);
}
EXPORT_SYMBOL(osal_iommu_attach_group);

hi_void osal_iommu_detach_group(void *domain, void *group)
{
    iommu_detach_group((struct iommu_domain*)domain, (struct iommu_group*)group);
}
EXPORT_SYMBOL(osal_iommu_detach_group);

hi_s32 osal_bus_set_iommu(void *bus, const void *ops)
{
    return bus_set_iommu((struct bus_type*)bus, (const struct iommu_ops*)ops);
}
EXPORT_SYMBOL(osal_bus_set_iommu);

hi_s32 osal_iommu_sva_device_init(void *dev)
{
    hi_s32 ret = 0;

    if (dev == NULL) {
        return -EFAULT;
    }

#ifndef AOS_LLVM_BUILD
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    ret = iommu_dev_enable_feature((struct device*)dev, IOMMU_DEV_FEAT_IOPF);
    if (ret != 0) {
        return ret;
    }
    ret = iommu_dev_enable_feature((struct device*)dev, IOMMU_DEV_FEAT_SVA);
#else
    ret = iommu_sva_device_init((struct device*)dev, IOMMU_SVA_FEAT_IOPF, UINT_MAX, NULL);
#endif
#else // #ifndef AOS_LLVM_BUILD
    ret = iommu_dev_enable_feature((struct device*)dev, IOMMU_DEV_FEAT_SVA);
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "iommu_dev_enable_feature IOMMU_DEV_FEAT_SVA ret= %d", ret);
        return ret;
    }

    ret = iommu_sva_device_init((struct device*)dev, IOMMU_DEV_FEAT_SVA, UINT_MAX, NULL);
#endif
    return ret;
}
EXPORT_SYMBOL(osal_iommu_sva_device_init);

hi_s32 osal_iommu_sva_device_deinit(void *dev)
{
    hi_s32 ret = 0;
    if (dev == NULL) {
        return -EFAULT;
    }
/* aoscore 只有5.10.0,在deinit时无需device_shutdow */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    ret = iommu_dev_disable_feature((struct device*)dev, IOMMU_DEV_FEAT_SVA);
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "iommu_dev_disable_feature IOMMU_DEV_FEAT_SVA failed, ret = %d", ret);
    }
#ifndef AOS_LLVM_BUILD /* aoscore不支持IOMMU_DEV_FEAT_IOPF */
    ret = iommu_dev_disable_feature((struct device*)dev, IOMMU_DEV_FEAT_IOPF);
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "iommu_dev_disable_feature IOMMU_DEV_FEAT_IOPF failed, ret = %d", ret);
    }
#endif  // #ifndef AOS_LLVM_BUILD
#else
    ret = iommu_sva_device_shutdown((struct device*)dev);
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "iommu_sva_device_shutdown failed, ret = %d", ret);
    }
#endif
    return ret;
}
EXPORT_SYMBOL(osal_iommu_sva_device_deinit);

void *osal_iommu_domain_alloc(void *bus)
{
    return iommu_domain_alloc((struct bus_type*)bus);
}
EXPORT_SYMBOL(osal_iommu_domain_alloc);

void osal_iommu_domain_free(void *domain)
{
    iommu_domain_free((struct iommu_domain*)domain);
}
EXPORT_SYMBOL(osal_iommu_domain_free);

int osal_iommu_domain_set_attr(void *domain, hi_s32 attr, void *data)
{
#ifndef AOS_LLVM_BUILD
    return -ENOTSUPP; /* iommu_domain_set_attr removed in 6.x */
#else
    return 0;
#endif
}
EXPORT_SYMBOL(osal_iommu_domain_set_attr);

void *osal_iommu_group_get(void *dev)
{
    return iommu_group_get((struct device*)dev);
}
EXPORT_SYMBOL(osal_iommu_group_get);

void osal_iommu_group_put(void *group)
{
    iommu_group_put((struct iommu_group *)group);
}
EXPORT_SYMBOL(osal_iommu_group_put);

hi_s32 osal_iommu_map(void *domain, hi_ulong iova, hi_ulong paddr, hi_ulong size, hi_s32 prot)
{
    return iommu_map((struct iommu_domain *)domain, iova, paddr, size, prot, GFP_KERNEL);
}
EXPORT_SYMBOL(osal_iommu_map);

size_t osal_iommu_unmap(void *domain, unsigned long iova, size_t size)
{
    return iommu_unmap((struct iommu_domain *)domain, iova, size);
}
EXPORT_SYMBOL(osal_iommu_unmap);

hi_s32 osal_iommu_sva_bind_device(void *dev, hi_s32 *pasid, hi_void** sva)
{
    hi_s32 ret = 0;

    if ((dev == NULL) || (pasid == NULL) || (sva == NULL)) {
        return -EFAULT;
    }

#ifdef AOS_LLVM_BUILD
    *sva = iommu_sva_bind_device((struct device*)dev, current->mm, pasid, IOMMU_DEV_FEAT_SVA, NULL);
    if (IS_ERR(*sva)) {
        ret = PTR_ERR(*sva);
    }
#else // #ifdef AOS_LLVM_BUILD
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    *sva = (hi_void *)iommu_sva_bind_device((struct device*)dev, current->mm);
    *pasid = (hi_s32)iommu_sva_get_pasid((struct iommu_sva *)*sva);
    if (IS_ERR(*sva)) {
        ret = PTR_ERR(*sva);
    }
#else // #if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    ret = iommu_sva_bind_device((struct device*)dev, current->mm, pasid, IOMMU_SVA_FEAT_IOPF, NULL);
    HI_UNUSED(sva);
#endif // #if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#endif // #ifdef AOS_LLVM_BUILD
    return ret;
}
EXPORT_SYMBOL(osal_iommu_sva_bind_device);

hi_void osal_iommu_sva_unbind_device(void *handle, hi_u32 ssid)
{
#ifdef AOS_LLVM_BUILD
    iommu_sva_unbind_device((struct iommu_sva*)handle, ssid);
#else // #ifdef AOS_LLVM_BUILD
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    HI_UNUSED(ssid);
    iommu_sva_unbind_device((struct iommu_sva*)handle);
#else
    HI_UNUSED(handle);
    HI_UNUSED(ssid);
#endif // #if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#endif // #ifdef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_iommu_sva_unbind_device);

hi_u64 osal_iommu_iova_to_phys(struct iommu_domain *domain, hi_u64 iova)
{
#ifdef AOS_LLVM_BUILD
    return 0;
#else
    return iommu_iova_to_phys(domain, iova);
#endif
}
EXPORT_SYMBOL(osal_iommu_iova_to_phys);

int osal_svm_get_pasid(pid_t vpid, int dev_id)
{
    return svm_get_pasid(vpid, dev_id);
}
EXPORT_SYMBOL(osal_svm_get_pasid);

typedef struct iommu_domain *(*svm_find_domain_fn)(char *smmu_name);

struct iommu_domain *osal_svm_get_dvpp_smmu_domain(hi_u32 idx)
{
#ifdef AOS_LLVM_BUILD
    return HI_NULL;
#else
    static svm_find_domain_fn find_fn;
    static int resolved;
    hi_char dvpp_smmu_name[SMMU_NAME_MAX_LEN] = { [0 ... SMMU_NAME_MAX_LEN - 1] = '\0' };
    hi_s32 ret;

    if (!resolved) {
        find_fn = (svm_find_domain_fn)compat_lookup_name("svm_find_domain_by_name");
        resolved = 1;
    }
    if (find_fn == HI_NULL) {
        return HI_NULL;
    }
    ret = snprintf_s(dvpp_smmu_name, SMMU_NAME_MAX_LEN - 1, SMMU_NAME_MAX_LEN - 2, "dvpp_smmu_%u", idx);
    if (ret < 0) {
        return HI_NULL;
    }
    return find_fn(dvpp_smmu_name);
#endif
}
EXPORT_SYMBOL(osal_svm_get_dvpp_smmu_domain);