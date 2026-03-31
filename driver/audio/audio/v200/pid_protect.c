/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/ascend_smmu.h>
#include <linux/of_address.h>
#include <linux/module.h>
#include "mkp_aio.h"
#include "hi_osal.h"
typedef osal_spinlock_t                        pid_spin_lock_s;
#define pid_spin_lock_init(lock)               osal_spin_lock_init(lock)
#define pid_spin_lock_destroy(lock)            osal_spin_lock_destroy(lock)
#define pid_spin_lock_irqsave(lock, flag)      osal_spin_lock_irqsave(lock, &(flag))
#define pid_spin_unlock_irqrestore(lock, flag) osal_spin_unlock_irqrestore(lock, &(flag))
pid_spin_lock_s g_pid_lock;
#define LEFT_SHIFT_NUM 16

hi_s32 g_cur_pid;
hi_s32 g_total_open_cnt;
#define AUDIO_CAP_WRAP_BASE_PA 0x400160000

static void dec_and_clear_pid(void)
{
    g_total_open_cnt--;
    if (g_total_open_cnt == 0) {
        g_cur_pid = 0;
    }
}

static hi_bool check_and_set_pid(hi_bool dev_open)
{
    hi_bool check_pid = HI_FALSE;

    if (g_cur_pid == 0) {
        g_cur_pid = current->tgid;
        check_pid = HI_TRUE;
    } else if (g_cur_pid == current->tgid) {
        check_pid = HI_TRUE;
    } else {
        check_pid = HI_FALSE;
    }

    if ((check_pid == HI_TRUE) && (dev_open == HI_TRUE)) {
        g_total_open_cnt++;
    }

    return check_pid;
}
hi_bool check_is_same_pid(hi_bool dev_open)
{
    unsigned long lockflag;
    pid_spin_lock_irqsave(&g_pid_lock, lockflag);
    hi_bool res = check_and_set_pid(dev_open);
    pid_spin_unlock_irqrestore(&g_pid_lock, lockflag);
    return res;
}
EXPORT_SYMBOL(check_is_same_pid);

void check_is_close(void)
{
    unsigned long lockflag;
    pid_spin_lock_irqsave(&g_pid_lock, lockflag);
    dec_and_clear_pid();
    pid_spin_unlock_irqrestore(&g_pid_lock, lockflag);
}
EXPORT_SYMBOL(check_is_close);

void check_pid_spin_lock_init(void)
{
    (void)pid_spin_lock_init(&g_pid_lock);
}
EXPORT_SYMBOL(check_pid_spin_lock_init);

void check_pid_spin_lock_destroy(void)
{
    (void)pid_spin_lock_destroy(&g_pid_lock);
}
EXPORT_SYMBOL(check_pid_spin_lock_destroy);

static hi_bool smmu_write_reg(uint64_t addr, uint32_t value)
{
    void *reg_va_addr = NULL;
    static const hi_u32 reg_size = 0x4;
    reg_va_addr = ioremap(addr, reg_size);
    if (reg_va_addr == NULL) {
        aio_err_trace("audio_drv_write_reg >> reg_va_addr ioremap failed!\n");
        return HI_FALSE;
    }
    writel(value, reg_va_addr);
    iounmap(reg_va_addr);
    reg_va_addr = NULL;
    return HI_TRUE;
}

static void smmu_read_reg(const void *pid_reg_base, unsigned int *value)
{
    if (pid_reg_base == NULL) {
        aio_err_trace("pid base reg is null\n");
        return;
    }
    *value = (*(const volatile td_u32 *)((td_u8 *)pid_reg_base));
    iounmap(pid_reg_base);
    pid_reg_base = NULL;
}

hi_s32 audio_config_smmu(void)
{
    hi_u32 sid = 0x1f;
    hi_u32 ssid;
    hi_u32 awuser_0 = 0;
    hi_u32 awaddr_ext = 0xf000; // 高位补齐
    ssid = svm_get_pasid(current->tgid, 0);
    awuser_0 += ssid << LEFT_SHIFT_NUM;
    awuser_0 += sid;
    void * aw_reg_base = ioremap(AUDIO_CAP_WRAP_BASE_PA + 0x3C28, 0x4);
    hi_u32 val = 0;
    smmu_read_reg(aw_reg_base, &val);
    if (val == awuser_0) {
        return HI_SUCCESS;
    }

    (hi_void)smmu_write_reg(AUDIO_CAP_WRAP_BASE_PA + 0x3C28, awuser_0); // 写
    (hi_void)smmu_write_reg(AUDIO_CAP_WRAP_BASE_PA + 0x3C38, awuser_0); // 读

    // config smmu high addr 34-52位
    (hi_void)smmu_write_reg(AUDIO_CAP_WRAP_BASE_PA + 0x3C20, awaddr_ext);
    (hi_void)smmu_write_reg(AUDIO_CAP_WRAP_BASE_PA + 0x3C24, awaddr_ext);

    (hi_void)smmu_write_reg(AUDIO_CAP_WRAP_BASE_PA + 0x3c2c, 0x00004055);
    (hi_void)smmu_write_reg(AUDIO_CAP_WRAP_BASE_PA + 0x3c30, 0x00000020);
    (hi_void)smmu_write_reg(AUDIO_CAP_WRAP_BASE_PA + 0x3c3c, 0x00004055);
    (hi_void)smmu_write_reg(AUDIO_CAP_WRAP_BASE_PA + 0x3c40, 0x00000020);
    return HI_SUCCESS;
}
EXPORT_SYMBOL(audio_config_smmu);

MODULE_LICENSE("GPL");
