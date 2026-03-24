/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-08-13
 */

#ifndef BBOX_PRODUCT_H
#define BBOX_PRODUCT_H

#include <linux/io.h>
#include "common/bbox_print.h"
#include "device/bbox_types.h"

#define FEATURE_OOM                     (1ULL << 0)     // oom: (mini, cloud, dc)
#define FEATURE_SPMI                    (1ULL << 1)     // spmi: (mini, minirc)
#define FEATURE_KERNEL_STAGE            (1ULL << 3)     // boot kernel dot: (mini, minirc, mdc)
#define FEATURE_BOOTCHECK_CDR           (1ULL << 10)    // bootcheck send data: cdr.(cloud)
#define FEATURE_BOOTCHECK_BIOS_STAGE    (1ULL << 11)    // bootcheck send data: bios stage.(mini, minirc, cloud)
#define FEATURE_BOOTCHECK_DDRDUMP       (1ULL << 12)    // bootcheck send data: bbox + klog.(minirc, mdc)
#define FEATURE_BOOTPARAM_CMDLINE       (1ULL << 20)    // bootparam from cmdline: (other, besides dc helper)
#define FEATURE_BOOTPARAM_DDR           (1ULL << 21)    // bootparam from ddr: (dc helper)

#if defined (BBOX_SOC_PLATFORM_MDC)
// MDC
#define BBOX_PRODUCT           "MDC"

/*
 * @brief           : get product feature
 * @return          : product feature bits
 */
static inline u64 bbox_get_feature(void)
{
    return (FEATURE_BOOTPARAM_CMDLINE | FEATURE_BOOTCHECK_DDRDUMP | FEATURE_KERNEL_STAGE);
}
#elif defined (BBOX_SOC_PLATFORM_MDC_V11)
#define BBOX_PRODUCT           "MDC"

#define PCIE_RC_EP_MD_ADDR  0xc0128000
#define PCIE_RC_EP_MD_MASK  0x8000
#define PCIE_RC_EP_MD_SIZE  4
#define PCIR_RC_MD          0

enum pcie_type {
    MODE_EP,
    MODE_RC,
    MODE_UNKNOWN,
};

/*
 * @brief           : get pcie mode
 * @return          : MODE_UNKNOWN      unknown
 *                    MODE_RC           rc
 *                    MODE_EP           ep
 */
static inline s32 bbox_get_pcie_mode(void)
{
    u32 mode, value;
    char *reg_addr = (char *)ioremap(PCIE_RC_EP_MD_ADDR, PCIE_RC_EP_MD_SIZE);
    BB_CHECK_PTR(reg_addr, return MODE_UNKNOWN, "Map pcie mode addr failed");
    value = readl(reg_addr);
    mode = value & PCIE_RC_EP_MD_MASK;
    iounmap((void __iomem *)reg_addr);
    return (mode == PCIR_RC_MD) ? MODE_RC : MODE_EP;
}

/*
 * @brief           : get product feature
 * @return          : product feature bits
 */
static inline u64 bbox_get_feature(void)
{
    s32 mode = bbox_get_pcie_mode();
    if (mode == MODE_EP) {
        return (FEATURE_OOM | FEATURE_BOOTPARAM_CMDLINE | FEATURE_KERNEL_STAGE |
                FEATURE_BOOTCHECK_DDRDUMP | FEATURE_BOOTCHECK_BIOS_STAGE);
    } else {
        return (FEATURE_BOOTPARAM_CMDLINE | FEATURE_KERNEL_STAGE |
                FEATURE_BOOTCHECK_DDRDUMP | FEATURE_BOOTCHECK_BIOS_STAGE);
    }
}
#elif defined (BBOX_SOC_PLATFORM_DC)
// DC, helper use dc
#define BBOX_PRODUCT           "DC"

#if defined BBOX_FEATURE_HELPER
/*
 * @brief           : get product feature
 * @return          : product feature bits
 */
static inline u64 bbox_get_feature(void)
{
    return (FEATURE_BOOTPARAM_DDR);
}
#else
static inline u64 bbox_get_feature(void)
{
    return (FEATURE_OOM | FEATURE_BOOTPARAM_CMDLINE);
}
#endif

#elif defined (BBOX_SOC_PLATFORM_CLOUD)
// CLOUD
#define BBOX_PRODUCT           "CLOUD"

/*
 * @brief           : get product feature
 * @return          : product feature bits
 */
static inline u64 bbox_get_feature(void)
{
    return (FEATURE_OOM | FEATURE_BOOTCHECK_CDR | FEATURE_BOOTPARAM_CMDLINE | FEATURE_BOOTCHECK_BIOS_STAGE);
}

#elif defined (BBOX_SOC_PLATFORM_MILAN)
// MILAN
#define BBOX_PRODUCT           "MILAN"

/*
 * @brief           : get product feature
 * @return          : product feature bits
 */
static inline u64 bbox_get_feature(void)
{
    return (FEATURE_OOM | FEATURE_BOOTCHECK_CDR | FEATURE_BOOTPARAM_CMDLINE | FEATURE_BOOTCHECK_BIOS_STAGE);
}

#elif defined (BBOX_SOC_PLATFORM_MILANR3)

#define BBOX_PRODUCT           "MILANR3"

#define PCIE_RC_EP_MD_ADDR  0xc0128000
#define PCIE_RC_EP_MD_MASK  0x8000
#define PCIE_RC_EP_MD_SIZE  4
#define PCIR_RC_MD          0

enum pcie_type {
    MODE_EP,
    MODE_RC,
    MODE_UNKNOWN,
};

/*
 * @brief           : get pcie mode
 * @return          : MODE_UNKNOWN      unknown
 *                    MODE_RC           rc
 *                    MODE_EP           ep
 */
static inline s32 bbox_get_pcie_mode(void)
{
    u32 mode, value;
    char *reg_addr = (char *)ioremap(PCIE_RC_EP_MD_ADDR, PCIE_RC_EP_MD_SIZE);
    BB_CHECK_PTR(reg_addr, return MODE_UNKNOWN, "Map pcie mode addr failed");
    value = readl(reg_addr);
    mode = value & PCIE_RC_EP_MD_MASK;
    iounmap((void __iomem *)reg_addr);
    return (mode == PCIR_RC_MD) ? MODE_RC : MODE_EP;
}

/*
 * @brief           : get product feature
 * @return          : product feature bits
 */
static inline u64 bbox_get_feature(void)
{
    s32 mode = bbox_get_pcie_mode();
    if (mode == MODE_EP) {
        return (FEATURE_OOM | FEATURE_BOOTPARAM_CMDLINE | FEATURE_KERNEL_STAGE |
                FEATURE_BOOTCHECK_BIOS_STAGE);
    } else {
        return (FEATURE_BOOTPARAM_CMDLINE | FEATURE_KERNEL_STAGE |
                FEATURE_BOOTCHECK_DDRDUMP | FEATURE_BOOTCHECK_BIOS_STAGE);
    }
}

#elif defined (BBOX_SOC_PLATFORM_MINI)
// MINI
#define BBOX_PRODUCT           "MINI"

#define PCIE_RC_EP_MD_ADDR  0x1100Ce088
#define PCIE_RC_EP_MD_MASK  0x8
#define PCIE_RC_EP_MD_SIZE  4
#define PCIR_RC_MD          0

enum pcie_type {
    MODE_EP,
    MODE_RC,
    MODE_UNKNOWN,
};

/*
 * @brief           : get pcie mode
 * @return          : MODE_UNKNOWN      unknown
 *                    MODE_RC           rc
 *                    MODE_EP           ep
 */
static inline s32 bbox_get_pcie_mode(void)
{
    u32 mode, value;
    char *reg_addr = (char *)ioremap(PCIE_RC_EP_MD_ADDR, PCIE_RC_EP_MD_SIZE);
    BB_CHECK_PTR(reg_addr, return MODE_UNKNOWN, "Map pcie mode addr failed");
    value = readl(reg_addr);
    mode = value & PCIE_RC_EP_MD_MASK;
    iounmap((void __iomem *)reg_addr);
    return (mode == PCIR_RC_MD) ? MODE_RC : MODE_EP;
}

/*
 * @brief           : get product feature
 * @return          : product feature bits
 */
static inline u64 bbox_get_feature(void)
{
    s32 mode = bbox_get_pcie_mode();
    if (mode == MODE_EP) {
        return (FEATURE_SPMI | FEATURE_OOM | FEATURE_BOOTPARAM_CMDLINE | FEATURE_KERNEL_STAGE |
                FEATURE_BOOTCHECK_BIOS_STAGE);
    } else {
        return (FEATURE_SPMI | FEATURE_BOOTPARAM_CMDLINE | FEATURE_KERNEL_STAGE |
                FEATURE_BOOTCHECK_DDRDUMP | FEATURE_BOOTCHECK_BIOS_STAGE);
    }
}

#else
// UNKNOWN
#define BBOX_PRODUCT           "UNKNOWN"

/*
 * @brief           : get product feature
 * @return          : product feature bits
 */
static inline u64 bbox_get_feature(void)
{
    return FEATURE_OOM | FEATURE_BOOTPARAM_CMDLINE;
}
#endif


/*
 * @brief           : check product feature is usable
 * @param [in]      : u64 feature           feature flag
 * @return          : true      usable;
 *                    false     unusable
 */
static inline bool bbox_check_feature(u64 feature)
{
    u64 product_feature = bbox_get_feature();
    return ((product_feature & feature) != 0) ? true : false;
}

#endif
