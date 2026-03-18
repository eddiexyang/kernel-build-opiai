/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2009-2022. All rights reserved.
 * Description: adec module driver
 * Author: Hisilicon multimedia software group
 * Create: 2009/6/19
 */

#include "adec.h"

#include "ot_osal.h"
#include "securec.h"

#include "dev_ext.h"
#include "mod_ext.h"
#include "proc_ext.h"
#include "valg_plat.h"
#include "valg_ext.h"
#include "aio_pub.h"
#include "pid_protect.h"
#ifdef CONFIG_OT_PLATFORM_V8
#include "ot_inner_common_adec.h"
#else
#include "ot_comm_adec_adapt.h"
#endif

#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
#include "adec_proc.h"
#endif

#include "hi_audio_adec.h"
#include "hi_comm_audio.h"

#ifdef AUDIO_UT_VCAST
#include <linux/device.h>
#endif

#define ADEC_STATE_STARTED  0
#define ADEC_STATE_STOPPING 1
#define ADEC_STATE_STOPPED  2

#ifndef umap_get_chn
#define umap_get_chn UMAP_GET_CHN
#endif

#ifndef umap_set_chn
#define umap_set_chn UMAP_SET_CHN
#endif
static osal_atomic_t g_adec_user_ref = OSAL_ATOMIC_INIT(0);
static osal_atomic_t g_adec_open_ref = OSAL_ATOMIC_INIT(0);
static td_u32 g_adec_state = ADEC_STATE_STOPPED;

static osal_dev_t *g_adec_umap_dev = TD_NULL;

adec_chn_ctx g_adec_chn[OT_ADEC_MAX_CHN_NUM];

static struct osal_semaphore g_adec_sem;

adec_chn_ctx *adec_get_chn_ctx(td_s32 chn_id)
{
    return &g_adec_chn[chn_id];
}

static td_s32 adec_create_chn(ot_adec_chn ad_chn, const ot_adec_chn_attr *chn_attr)
{
    adec_chn_ctx *adec_chn = TD_NULL;
    td_s32 ret;

    adec_check_chn_return(ad_chn);
    adec_check_null_ptr_return(chn_attr);

    adec_chn = &g_adec_chn[ad_chn];

    ret = memcpy_s(&adec_chn->chn_attr, sizeof(adec_chn->chn_attr), chn_attr, sizeof(*chn_attr));
    if (ret != EOK) {
        adec_err_trace("adec_chn %d chn_attr memcpy_s fail, ret = 0x%x.\n", ad_chn, (td_u32)ret);
        return OT_ERR_ADEC_ILLEGAL_PARAM;
    }

    adec_chn->created = TD_TRUE;
    return TD_SUCCESS;
}

static td_s32 adec_destroy_chn(ot_adec_chn ad_chn)
{
    adec_chn_ctx *adec_chn = TD_NULL;

    adec_check_chn_return(ad_chn);

    adec_chn = &g_adec_chn[ad_chn];

    adec_chn->created = TD_FALSE;
    return TD_SUCCESS;
}

static td_s32 adec_set_dbg_info(ot_adec_chn ad_chn, const adec_dbg_info *dbg_info)
{
    adec_chn_ctx *adec_chn = TD_NULL;

    adec_check_chn_return(ad_chn);
    adec_check_null_ptr_return(dbg_info);

    adec_chn = &g_adec_chn[ad_chn];

    adec_chn->dbg_info.get_cnt = dbg_info->get_cnt;
    adec_chn->dbg_info.put_cnt = dbg_info->put_cnt;
    adec_chn->dbg_info.send_cnt = dbg_info->send_cnt;
    adec_chn->dbg_info.ori_send_cnt = dbg_info->ori_send_cnt;
    adec_chn->dbg_info.g726_rate = dbg_info->g726_rate;
    adec_chn->dbg_info.adpcm_type = dbg_info->adpcm_type;
    adec_chn->dbg_info.stream = dbg_info->stream;

    return TD_SUCCESS;
}

static long adec_do_ioctl(unsigned int cmd, unsigned long ul_arg, void *private_data)
{
    td_s32 chn_id = umap_get_chn(private_data);
    td_uintptr_t arg = (td_uintptr_t)ul_arg;

#ifdef CONFIG_OT_AUDIO_ONLY_DRIVER_SUPPORT
    /* 只支持驱动的情况下无须开放ioctl，直接返回 */
    return TD_FAILURE;
#endif

    switch (cmd) {
        case ADEC_BINDCHN2FD_CTRL: {
            adec_check_null_ptr_return(arg);
            umap_set_chn(private_data, *((td_u32 *)arg));
            break;
        }
        case ADEC_CREATE_CHN_CTRL: {
            return adec_create_chn(chn_id, (ot_adec_chn_attr *)arg);
        }
        case ADEC_DESTROY_CHN_CTRL: {
            return adec_destroy_chn(chn_id);
        }
        case ADEC_SET_DBGINFO_CTRL: {
            return adec_set_dbg_info(chn_id, (adec_dbg_info *)arg);
        }

        default: {
            adec_err_trace("adec_do_ioctl: ERR IOCTL CMD 0x%x, NR:%u\n", cmd, _IOC_NR(cmd));
            return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

#ifdef CONFIG_COMPAT
static long adec_compat_ioctl(unsigned int cmd, unsigned long arg_org, void *private_data)
{
    td_s32 ret;
    td_uintptr_t arg = (td_uintptr_t)arg_org;

    switch (cmd) {
        case ADEC_CREATE_CHN_CTRL: {
            ot_adec_chn_attr *attr = (ot_adec_chn_attr *)arg;
            OT_COMPAT_POINTER(attr->value, td_void *);
            break;
        }

        default:
            break;
    }

    osal_atomic_inc_return(&g_adec_user_ref);
    ret = (td_s32)adec_do_ioctl(cmd, arg_org, private_data);
    osal_atomic_dec_return(&g_adec_user_ref);

    return ret;
}
#endif

static unsigned int adec_poll(osal_poll_t *poll, void *data)
{
    ot_unused(poll);
    ot_unused(data);
    return TD_SUCCESS;
}

static long adec_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    int ret;
    if (check_is_same_pid(TD_FALSE) != TD_TRUE) {
        return OT_ERR_ADEC_NOT_PERM;
    }
    td_s32 chn_id = umap_get_chn(private_data);
    if ((chn_id < 0) && (cmd != ADEC_BINDCHN2FD_CTRL)) {
        return OT_ERR_ADEC_NOT_PERM;
    }

    if (g_adec_state != ADEC_STATE_STARTED) {
        adec_warn_trace("MPP sys not init!\n");
        return OT_ERR_ADEC_NOT_READY;
    }

    osal_atomic_inc_return(&g_adec_user_ref);
    ret = (int)adec_do_ioctl(cmd, arg, private_data);
    osal_atomic_dec_return(&g_adec_user_ref);

    return ret;
}

static int adec_open(void *private_data)
{
    td_s32 i;
    if (osal_down(&g_adec_sem) != 0) {
        adec_err_trace("adec open semaphore down fail!\n");
        return TD_FAILURE;
    }
    ot_unused(private_data);
    if (osal_atomic_read(&g_adec_open_ref) >= MAX_OPEN_CNT) {
        osal_up(&g_adec_sem);
        return TD_FAILURE;
    }
    if (check_is_same_pid(TD_TRUE) != TD_TRUE) {
        osal_up(&g_adec_sem);
        return TD_FAILURE;
    }
    if (osal_atomic_inc_return(&g_adec_open_ref) != 1) {
        osal_up(&g_adec_sem);
        adec_debug_trace(" adec already initialization when first open\n");
        return TD_SUCCESS;
    }

    for (i = 0; i < OT_ADEC_MAX_CHN_NUM; i++) {
        (td_void)memset_s(&g_adec_chn[i], sizeof(adec_chn_ctx), 0, sizeof(adec_chn_ctx));
        g_adec_chn[i].dbg_info.g726_rate = OT_G726_BUTT;
        g_adec_chn[i].dbg_info.adpcm_type = OT_ADPCM_TYPE_BUTT;
    }

    g_adec_state = ADEC_STATE_STARTED;
    osal_up(&g_adec_sem);
    return TD_SUCCESS;
}

static int adec_close(void *private_data)
{
    td_s32 chn_id = umap_get_chn(private_data);
    if (osal_down(&g_adec_sem) != 0) {
        adec_err_trace("adec close semaphore down fail!\n");
        return TD_FAILURE;
    }
    if (osal_atomic_dec_return(&g_adec_open_ref) == 0) {
        g_adec_state = ADEC_STATE_STOPPED;
    }
    (void)check_is_close();
    osal_up(&g_adec_sem);
    return TD_SUCCESS;
}

static struct osal_fileops g_adec_fops = {
    .module = THIS_MODULE,
    .open = adec_open,
    .release = adec_close,
    .unlocked_ioctl = adec_ioctl,
    .poll = adec_poll,
#ifdef CONFIG_COMPAT
    .compat_ioctl = adec_compat_ioctl
#endif
};

/* ADEC 的MPP业务初始化函数 */
static td_s32 adec_init(void *p)
{
    ot_unused(p);

    return TD_SUCCESS;
}

/* ADEC 的MPP业务去初始化函数 */
static td_void adec_exit(void)
{
    return;
}

static td_void adec_notify(mod_notice_id notice)
{
    ot_unused(notice);
    return;
}

static td_void adec_query_state(mod_state *state)
{
    if (osal_atomic_read(&g_adec_user_ref) == 0) {
        *state = MOD_STATE_FREE;
    } else {
        *state = MOD_STATE_BUSY;
    }
    return;
}

static td_u32 adec_get_ver_magic(td_void)
{
    return VERSION_MAGIC;
}

static umap_module g_adec_module = {
    .mod_id = OT_ID_ADEC,
    .mod_name = "adec",

    .pfn_init = adec_init,
    .pfn_exit = adec_exit,
    .pfn_query_state = adec_query_state,
    .pfn_notify = adec_notify,
    .pfn_ver_checker = adec_get_ver_magic,

    .data = TD_NULL,
};

int adec_module_init(void)
{
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_proc_entry_t *sub_proc = TD_NULL;

    /* create proc interface */
    sub_proc = osal_create_proc_entry(PROC_ENTRY_ADEC, TD_NULL);
    if (sub_proc == TD_NULL) {
        adec_err_trace("adec create proc failed\n");
        return TD_FAILURE;
    }
    sub_proc->read = adec_proc_show;
#endif
    td_s32 ret;
    g_adec_umap_dev = osal_createdev(UMAP_DEVNAME_ADEC_BASE);
    if (g_adec_umap_dev == TD_NULL) {
        adec_err_trace("adec create dev failed\n");
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_ADEC, TD_NULL);
#endif
        return TD_FAILURE;
    }
    g_adec_umap_dev->fops = &g_adec_fops;
    g_adec_umap_dev->minor = UMAP_ADEC_MINOR_BASE;
    if (osal_registerdevice(g_adec_umap_dev) != TD_SUCCESS) {
        adec_err_trace("adec register dev failed\n");
        (td_void)osal_destroydev(g_adec_umap_dev);
        g_adec_umap_dev = TD_NULL;
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_ADEC, TD_NULL);
#endif
        return TD_FAILURE;
    }

    if (cmpi_register_module(&g_adec_module)) {
        adec_err_trace("adec register module failed\n");
        osal_deregisterdevice(g_adec_umap_dev);
        (td_void)osal_destroydev(g_adec_umap_dev);
        g_adec_umap_dev = TD_NULL;
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_ADEC, TD_NULL);
#endif
        return TD_FAILURE;
    }
    ret = osal_atomic_init(&g_adec_user_ref);
    if (ret < 0) {
        cmpi_unregister_module(OT_ID_ADEC);
        osal_deregisterdevice(g_adec_umap_dev);
        (td_void)osal_destroydev(g_adec_umap_dev);
        g_adec_umap_dev = TD_NULL;
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_ADEC, TD_NULL);
#endif
        return TD_FAILURE;
    }
    osal_atomic_set(&g_adec_user_ref, 0);
    // 初始化adec初始化引用计数
    ret = osal_atomic_init(&g_adec_open_ref);
    if (ret < 0) {
        osal_atomic_destroy(&g_adec_user_ref);
        cmpi_unregister_module(OT_ID_ADEC);
        osal_deregisterdevice(g_adec_umap_dev);
        (td_void)osal_destroydev(g_adec_umap_dev);
        g_adec_umap_dev = TD_NULL;
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_ADEC, TD_NULL);
#endif
        adec_err_trace("adec open_ref atomic init failed\n");
        return TD_FAILURE;
    }
    osal_atomic_set(&g_adec_open_ref, 0);
    ret = osal_sema_init(&g_adec_sem, 1);
    if (ret < 0) {
        osal_atomic_destroy(&g_adec_open_ref);
        osal_atomic_destroy(&g_adec_user_ref);
        cmpi_unregister_module(OT_ID_ADEC);
        osal_deregisterdevice(g_adec_umap_dev);
        (td_void)osal_destroydev(g_adec_umap_dev);
        g_adec_umap_dev = TD_NULL;
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry(PROC_ENTRY_ADEC, TD_NULL);
#endif
        adec_err_trace("adec osal g_adec_sem init fail\n");
        return TD_FAILURE;
    }
    adec_info_trace("load adec.ko ....OK!\n");
    return TD_SUCCESS;
}

void adec_module_exit(void)
{
    osal_sema_destroy(&g_adec_sem);
    osal_atomic_destroy(&g_adec_open_ref);
    osal_atomic_destroy(&g_adec_user_ref);

    cmpi_unregister_module(OT_ID_ADEC);

    if (g_adec_umap_dev != TD_NULL) {
        osal_deregisterdevice(g_adec_umap_dev);
        (td_void)osal_destroydev(g_adec_umap_dev);
        g_adec_umap_dev = TD_NULL;
    }

#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_remove_proc_entry(PROC_ENTRY_ADEC, TD_NULL);
#endif
    adec_info_trace("unload adec.ko ....OK!\n");
    return;
}
