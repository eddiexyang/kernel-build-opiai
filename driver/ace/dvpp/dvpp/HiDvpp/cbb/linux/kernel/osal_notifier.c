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

#include <linux/notifier.h>
#include <linux/oom.h>
#ifndef AOS_LLVM_BUILD
#include <linux/kdebug.h>
#include <linux/suspend.h>
#include <linux/reboot.h>
#else
#include <linux/export.h>
#endif // #ifndef AOS_LLVM_BUILD

hi_s32 osal_register_die_notifier(void *nb)
{
#ifdef AOS_LLVM_BUILD
    return 0;
#else // #ifdef AOS_LLVM_BUILD
    return register_die_notifier((struct notifier_block*)nb);
#endif // #ifdef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_register_die_notifier);

hi_s32 osal_unregister_die_notifier(void *nb)
{
#ifdef AOS_LLVM_BUILD
    return 0;
#else // #ifdef AOS_LLVM_BUILD
    return unregister_die_notifier((struct notifier_block*)nb);
#endif // #ifdef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_unregister_die_notifier);

hi_s32 osal_atomic_notifier_chain_register(void *nh, void *n)
{
#ifdef AOS_LLVM_BUILD
    return atomic_notifier_chain_register((struct AtomicNotifierHead*)nh, (struct notifier_block*)n);
#else // #ifdef AOS_LLVM_BUILD
    return atomic_notifier_chain_register((struct atomic_notifier_head*)nh, (struct notifier_block*)n);
#endif // #ifdef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_atomic_notifier_chain_register);

hi_s32 osal_atomic_notifier_chain_unregister(void *nh, void *n)
{
#ifdef AOS_LLVM_BUILD
    return atomic_notifier_chain_unregister((struct AtomicNotifierHead*)nh, (struct notifier_block*)n);
#else // #ifdef AOS_LLVM_BUILD
    return atomic_notifier_chain_unregister((struct atomic_notifier_head*)nh, (struct notifier_block*)n);
#endif // #ifdef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_atomic_notifier_chain_unregister);

hi_s32 osal_register_hisi_oom_notifier(void *nb)
{
#ifdef AOS_LLVM_BUILD
    return 0;
#else // #ifdef AOS_LLVM_BUILD
    return register_hisi_oom_notifier((struct notifier_block*)nb);
#endif // #ifdef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_register_hisi_oom_notifier);

hi_s32 osal_unregister_hisi_oom_notifier(void *nb)
{
#ifdef AOS_LLVM_BUILD
    return 0;
#else // #ifdef AOS_LLVM_BUILD
    return unregister_hisi_oom_notifier((struct notifier_block*)nb);
#endif // #ifdef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_unregister_hisi_oom_notifier);

hi_s32 osal_register_pm_notifier(void *nb)
{
#ifdef AOS_LLVM_BUILD
    return 0;
#else // #ifdef AOS_LLVM_BUILD
    return register_pm_notifier((struct notifier_block*)nb);
#endif // #ifdef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_register_pm_notifier);

hi_s32 osal_unregister_pm_notifier(void *nb)
{
#ifdef AOS_LLVM_BUILD
    return 0;
#else // #ifdef AOS_LLVM_BUILD
    return unregister_pm_notifier((struct notifier_block*)nb);
#endif // #ifdef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_unregister_pm_notifier);

hi_s32 osal_register_reboot_notifier(void *nb)
{
#ifdef AOS_LLVM_BUILD
    return 0;
#else // #ifdef AOS_LLVM_BUILD
    return register_reboot_notifier((struct notifier_block*)nb);
#endif // #ifdef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_register_reboot_notifier);

hi_s32 osal_unregister_reboot_notifier(void *nb)
{
#ifdef AOS_LLVM_BUILD
    return 0;
#else // #ifdef AOS_LLVM_BUILD
    return unregister_reboot_notifier((struct notifier_block*)nb);
#endif // #ifdef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_unregister_reboot_notifier);