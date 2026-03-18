/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description:define aio init source file
 * Author: Hisilicon multimedia software group
 * Create: 2019/12/25
 */

#include <linux/of_platform.h>
#include "ot_aio_mod_init.h"
#include "hi_comm_audio.h"
#include "asp_misc.h"
#include "asp_dma.h"
#include "mkp_aio.h"
#include "pid_protect.h"
EXPORT_SYMBOL(aiao_get_acodec_base);

static int ot_aiao_probe(struct platform_device *pdev)
{
    if (pdev == TD_NULL) {
        dev_err(&pdev->dev, "pdev is NULL\n");
        return TD_FAILURE;
    }

    int ret;
    ret = asp_misc_init(pdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "asp misc init fail in ot aiao probe\n");
        return ret;
    }

    (void)asp_dma_init(pdev);

    ret = aiao_module_init();
    if (ret != 0) {
        dev_err(&pdev->dev, "aiao moudle init fail in ot aiao probe\n");
        return ret;
    }
    check_pid_spin_lock_init();
    return 0;
}

static int ot_aiao_remove(struct platform_device *pdev)
{
    int ret;

    ot_unused(pdev);

    ret = asp_dma_remove(pdev);
    if (ret != 0) {
        dev_err(&pdev->dev, "asp dma remove fail in ot aio remove\n");
        return ret;
    }

    (void)asp_misc_exit();
    aiao_module_exit();
    check_pid_spin_lock_destroy();
    return 0;
}

/**
 * @brief: asp驱动向系统注册的休眠函数
 * @param [in] struct device *dev: 设备指针
 * @return : 成功返回0
 */
static int asp_suspend(struct device *dev)
{
    asp_do_suspend();
    aio_info_trace("asp suspend successfully!\n");

    return 0;
}

/**
 * @brief: asp驱动向系统注册的唤醒函数
 * @param [in] struct device *dev: 设备指针
 * @return : 成功返回0
 */
static int asp_resume(struct device *dev)
{
    asp_do_resume();
    aio_info_trace("asp resume successfully!\n");

    return 0;
}

SIMPLE_DEV_PM_OPS(asp_pm_ops, asp_suspend, asp_resume);
static const struct of_device_id g_ot_aiao_match[] = {
    { .compatible = "vendor,aiao" },
    {},
};
MODULE_DEVICE_TABLE(of, g_ot_aiao_match);

static struct platform_driver g_ot_aiao_driver = {
    .probe = ot_aiao_probe,
    .remove = ot_aiao_remove,
    .driver = {
        .name = "ot_aiao",
        .of_match_table = g_ot_aiao_match,
        .pm = &asp_pm_ops,
    },
};
#ifndef AUDIO_UT_VCAST
osal_module_platform_driver(g_ot_aiao_driver);
#endif

MODULE_LICENSE("GPL");

