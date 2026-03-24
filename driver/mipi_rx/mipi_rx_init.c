/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: mipi init function
 * Author: Hisilicon multimedia software group
 * Create: 2023-03-08
 */

#include <linux/types.h>
#include "hi_osal.h"
#include "mipi_rx_type.h"
#include "hi_type.h"
#include "securec.h"
#include "hi_mipi_rx.h"
#include "hi_mipi_rx_mod_init.h"
#include "mipi_rx_hal.h"
#include "slvs_hal.h"
#include "hi_debug.h"
#include "mipi_rx_proc.h"
#include "mipi_rx_config.h"
#include "mipi_rx_operate.h"
#ifdef CAMERAV2_UT_TEST
#include "mipi_rx_utest_stub.h"
#endif
/* macro definition */
#define MIPI_RX_DEV_NAME  "hi_mipi_rx"
#define MIPI_RX_PROC_NAME "mipi_rx"

#define CONFIG_OT_PROC_SHOW_SUPPORT
#define OT_MEDIA_DYNAMIC_MINOR 255

static osal_dev_t *g_mipi_rx_dev = NULL;

osal_mutex_t g_mipi_mutex;

mipi_dev_ctx_t g_mipi_dev_ctx;

osal_spinlock_t g_mipi_ctx_spinlock;

void mipi_rx_set_irq_num(unsigned int irq_num)
{
    mipi_rx_drv_set_irq_num(irq_num);
}

void slvs_set_irq_num(unsigned int irq_num)
{
    slvs_drv_set_irq_num(irq_num);
}

void mipi_rx_set_regs(const void *regs)
{
    mipi_rx_drv_set_regs((mipi_rx_regs_type_t *)regs);
}

void slvs_ec_set_regs(const void *regs)
{
    slvs_ec_drv_set_regs((slvs_regs_type_t *)regs);
}

#ifdef CONFIG_COMPAT
static long mipi_rx_compat_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    return mipi_rx_ioctl(cmd, arg, private_data);
}
#endif

#ifdef CONFIG_OT_PROC_SHOW_SUPPORT

static int proc_show(const osal_proc_entry_t* s)
{
    mipi_rx_proc_show(s);

    slvs_proc_show(s);

    return 0;
}
#endif

/**
 * @brief: mipi suspend 对外接口函数
 * @param: 无
 * @return : 休眠成功为0，失败为非0
 */
hi_s32 mipi_suspend(void)
{
    hi_s32 ret = HI_SUCCESS;
    ret = mipi_do_suspend();
    if (ret != 0) {
        mipi_rx_err_trace("mipi rx suspend failed!\n");
    } else {
        mipi_rx_info_trace("mipi rx suspend successfully!\n");
    }
    return ret;
}
EXPORT_SYMBOL(mipi_suspend);

/**
 * @brief: mipi resume 对外接口函数
 * @param: 无
 * @return : 唤醒成功为0，失败为非0
 */
hi_s32 mipi_resume(void)
{
    hi_s32 ret = HI_SUCCESS;
    ret = mipi_do_resume();
    if (ret != 0) {
        mipi_rx_err_trace("mipi rx resume failed!\n");
    } else {
        mipi_rx_info_trace("mipi rx resume successfully!\n");
    }
    return ret;
}
EXPORT_SYMBOL(mipi_resume);

static int mipi_rx_init(void)
{
    int ret;
    hi_u32 i;

    ret = osal_mutex_init(&g_mipi_mutex);
    if (ret < 0) {
        mipi_rx_err_trace("mutex init fail!\n");
        goto fail0;
    }

    ret = osal_spin_lock_init(&g_mipi_ctx_spinlock);
    if (ret < 0) {
        mipi_rx_err_trace("spin lock init fail!\n");
        goto fail1;
    }

    osal_spin_lock(&g_mipi_ctx_spinlock);

    g_mipi_dev_ctx.lane_divide_mode = LANE_DIVIDE_MODE_BUTT;
    for (i = 0U; i < MIPI_RX_MAX_DEV_NUM; i++) {
        g_mipi_dev_ctx.dev_cfged[i] = FALSE;
        g_mipi_dev_ctx.mipi_clock_status[i] = DISABLE;
        g_mipi_dev_ctx.reset_mipi_status[i] = RESET;
    }
    for (i = SLVS_DEV_NUM_START; i < (SLVS_DEV_NUM_START + SLVS_MAX_DEV_NUM); i++) {
        g_mipi_dev_ctx.dev_cfged[i] = FALSE;
        g_mipi_dev_ctx.slvs_clock_status[i] = DISABLE;
        g_mipi_dev_ctx.reset_slvs_status[i] = RESET;
    }
    for (i = 0U; i < SNS_MAX_CLK_SOURCE_NUM; i++) {
        g_mipi_dev_ctx.sns_clk_config_cfged[i] = FALSE;
        g_mipi_dev_ctx.sns_clk_source_cfged[i] = DISABLE;
    }
    for (i = 0U; i < SNS_MAX_RST_SOURCE_NUM; i++) {
        g_mipi_dev_ctx.sns_rst_source_cfged[i] = RESET;
    }
    for (i = 0U; i < COMBO_DEV_MAX_NUM; i++) {
        (void)memset_s(&g_mipi_dev_ctx.combo_dev_attr[i], sizeof(combo_dev_attr_t),
            0, sizeof(combo_dev_attr_t));
    }
    g_mipi_dev_ctx.hs_mode_cfged = FALSE;

    osal_spin_unlock(&g_mipi_ctx_spinlock);

    ret = mipi_rx_drv_init();
    if (ret < 0) {
        mipi_rx_err_trace("mipi_rx_drv_init fail!\n");
        goto fail2;
    }
    ret = slvs_drv_init();
    if (ret < 0) {
        mipi_rx_err_trace("slvs_drv_init fail!\n");
        goto fail3;
    }

    return 0;

fail3:
    mipi_rx_drv_exit();
fail2:
    osal_spin_lock_destroy(&g_mipi_ctx_spinlock);
fail1:
    osal_mutex_destroy(&g_mipi_mutex);
fail0:
    return -1;
}

static void mipi_rx_exit(void)
{
    mipi_rx_drv_exit();
    slvs_rx_drv_exit();
    osal_spin_lock_destroy(&g_mipi_ctx_spinlock);
    osal_mutex_destroy(&g_mipi_mutex);
}

static int mipi_rx_open(void *private_data)
{
    mipi_rx_unused(private_data);
    return 0;
}

static int mipi_rx_release(void *private_data)
{
    mipi_rx_unused(private_data);
    return 0;
}

static osal_fileops_t mipi_rx_fops = {
    .module = THIS_MODULE,
    .open = mipi_rx_open,
    .release = mipi_rx_release,
    .unlocked_ioctl = mipi_rx_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = mipi_rx_compat_ioctl,
#endif
};

int mipi_rx_mod_init(void)
{
    int ret;
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_proc_entry_t *mipi_rx_proc_entry = NULL;
#endif

    ret = init_mipi_rx_user_ref();
    if (ret != HI_SUCCESS) {
        mipi_rx_err_trace("osal_atomic_init failed !\n");
        goto fail0;
    }

    g_mipi_rx_dev = osal_createdev(MIPI_RX_DEV_NAME);
    if (g_mipi_rx_dev == NULL) {
        mipi_rx_err_trace("create mipi_rx device failed!\n");
        goto fail1;
    }

    g_mipi_rx_dev->fops = &mipi_rx_fops;
    g_mipi_rx_dev->minor = OT_MEDIA_DYNAMIC_MINOR;

    ret = osal_registerdevice(g_mipi_rx_dev);
    if (ret < 0) {
        mipi_rx_err_trace("register mipi_rx device failed!\n");
        goto fail2;
    }

#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    mipi_rx_proc_entry = osal_create_proc_entry(MIPI_RX_PROC_NAME, NULL);
    if (mipi_rx_proc_entry == NULL) {
        mipi_rx_err_trace("create mipi_rx proc(%s) failed!\n", MIPI_RX_PROC_NAME);
        goto fail3;
    }
    mipi_rx_proc_entry->read = proc_show;
    mipi_rx_proc_entry->write = NULL;
#endif

    ret = mipi_rx_init();
    if (ret < 0) {
        mipi_rx_err_trace("mipi_rx_init failed!\n");
        goto fail4;
    }

    mipi_rx_info_trace("load mipi_rx.ko ....OK!\n");
    return 0;

fail4:
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_remove_proc_entry(MIPI_RX_PROC_NAME, NULL);
fail3:
#endif
    osal_deregisterdevice(g_mipi_rx_dev);
fail2:
    (void)osal_destroydev(g_mipi_rx_dev);
    g_mipi_rx_dev = NULL;
fail1:
    destory_mipi_rx_user_ref();
fail0:
    mipi_rx_err_trace("load mipi_rx.ko ....failed!\n");
    return -1;
}

void mipi_rx_mod_exit(void)
{
    mipi_rx_exit();

#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_remove_proc_entry(MIPI_RX_PROC_NAME, NULL);
#endif

    osal_deregisterdevice(g_mipi_rx_dev);

    (void)osal_destroydev(g_mipi_rx_dev);
    g_mipi_rx_dev = NULL;
    destory_mipi_rx_user_ref();
    mipi_rx_info_trace("unload mipi_rx.ko ....OK!\n");
}