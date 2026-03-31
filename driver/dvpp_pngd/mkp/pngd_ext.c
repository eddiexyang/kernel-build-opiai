/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
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
 * Create: 2020-4-1
 */

#include <linux/sched.h>
#include <linux/module.h>

#include "hi_type.h"
#include "hi_common.h"
#include "hi_debug.h"
#include "mkp_pngd.h"
#include "dev_ext.h"
#include "sys_ext.h"
#include "pngd.h"
#include "proc_ext.h"
#include "hi_version.h"
#include "devdrv_manager_common.h"
#include "pngd_intf.h"
#include "pngd_ext.h"
#include "vmngd.h"
#include "dvpp_comm_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif // end of #ifdef __cplusplus

struct osal_semaphore g_pngd_sem;

static osal_dev_t *g_pngd_umapd;
hi_u32 g_created_pngd_chn_num = 0;
hi_u32 g_pngd_state = PNGD_STATE_STOPED;
osal_atomic_t g_pngd_user_ref = OSAL_ATOMIC_INIT(0);

// 注意，初始化必须按照ioc_nr_pngd中成员次序排列
static pngd_case g_pngd_case[IOC_NR_PNGD_BUTT] = {
    { PNGD_CHN_CREATE_CTRL, pngd_create_chn },
    { PNGD_CHN_DESTROY_CTRL, pngd_destory_chn },
    { PNGD_CHN_SENDSTREAM_CTRL, pngd_send_chn_stream },
    { PNGD_CHN_GETIMAGE_CTRL, pngd_get_chn_image },
    { PNGD_CHN_SET_ACL_CTRL, pngd_set_acl_param },
};

int pngd_open(void *data)
{
    UMAP_SET_CHN(data, -1);
    return 0;
}

int pngd_close(const void * const data)
{
    hi_s32 device_chan_id = 0;
    hi_s32 device_id = 0;
    hi_s32 chn_id = 0;

    chn_id = UMAP_GET_CHN(data);
    device_id = trans_pngd_chan_id_to_device_id(chn_id);
    device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);

    PNGD_INFO_TRACE("pngd_fops release: device %d chn %d tgid:%d pid:%d",
                    device_id, device_chan_id, current->tgid, current->pid);

    if ((chn_id < 0) || (chn_id >= INTERNAL_PNGD_MAX_CHN_NUM)) {
        return HI_SUCCESS;
    }

    pngd_destroy(chn_id);
    return 0;
}

long pngd_ioctl(unsigned int cmd, unsigned long para, void *private_data)
{
    hi_s32 ret;
    hi_u32 idex;
    hi_uintptr_t arg = (hi_uintptr_t)para;
    PNGD_HANDLER handler = HI_NULL;

    if (CHIP_SUPPORT_DEC() == 0) {
        PNGD_ERR_TRACE(" CHIP unsupport DEC. cmd:0x%x\n", cmd);
        return HI_ERR_PNGD_SYS_NOT_READY;
    }
    if (g_pngd_state != PNGD_STATE_STARTED) {
        PNGD_ERR_TRACE(" pngd_state:%u is not start. cmd:0x%x\n", g_pngd_state, cmd);
        return HI_ERR_PNGD_SYS_NOT_READY;
    }
    if (private_data == HI_NULL) {
        PNGD_ERR_TRACE(" private_data is null. cmd:0x%x\n", cmd);
        return HI_ERR_PNGD_NULL_PTR;
    }

    idex = _IOC_NR(cmd);
    if (idex >= IOC_NR_PNGD_BUTT) {
        PNGD_ERR_TRACE("Error IO ctrl cmd 0x%x!\n", cmd);
        return HI_ERR_PNGD_ILLEGAL_PARAM;
    }

    osal_atomic_inc_return(&g_pngd_user_ref);
    if (g_pngd_case[idex].cmd == cmd) {
        handler = g_pngd_case[idex].handler;
    }

    if (handler != HI_NULL) {
        ret = handler(arg, private_data);
    } else {
        ret = HI_ERR_PNGD_SYS_ERROR;
        PNGD_INFO_TRACE("Error IO ctrl cmd 0x%x!\n", cmd);
    }

    osal_atomic_dec_return(&g_pngd_user_ref);
    return ret;
}

#ifdef CONFIG_COMPAT
/**********************************************************************************
linux系统下，如果是内核态64位、用户态32位系统架构的应用场景，指针变量在内核态和用户态的长度是不一样的，
在内核态是64bit长度，用户态是32bit长度，如果用户态指针指向的内存里面的数据需要与内核态有交互，则需要把
用户态指针变量传递到内核态之后把高32bit野值清0，还原出一个真正的用户态指针. COMPAT_POINTER就是做这个事的.
***********************************************************************************/
long pngd_compat_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    if (g_pngd_state != PNGD_STATE_STARTED) {
        return HI_ERR_PNGD_SYS_NOT_READY;
    }

    switch (cmd) {
        case PNGD_CHN_SENDSTREAM_CTRL: {
            pngd_send_stream_info *pngd_send_stream = (pngd_send_stream_info *)(hi_uintptr_t)arg;
            if (pngd_send_stream != HI_NULL) {
                COMPAT_POINTER(pngd_send_stream->stream.addr, hi_u8 *);
            }
            break;
        }

        case PNGD_CHN_GETIMAGE_CTRL: {
            pngd_get_image_info *pngd_get_image = (pngd_get_image_info *)(hi_uintptr_t)arg;
            if (pngd_get_image != HI_NULL) {
                COMPAT_POINTER(pngd_get_image->png_pic_info, hi_pic_info *);
                COMPAT_POINTER(pngd_get_image->stream, hi_img_stream *);
            }
            break;
        }

        default: {
            break;
        }
    }

    return pngd_ioctl(cmd, arg, private_data);
}
#endif

static struct osal_fileops pngd_fops = {
    .module = THIS_MODULE, // open时在osal层会通过try_module_get(coat_dev->osal_dev.fops->module)增加引用计数
    .open = pngd_open,
    .unlocked_ioctl = pngd_ioctl,
    .release = pngd_close,
    .mmap = NULL,
    .poll = NULL,
#ifdef CONFIG_COMPAT
    .compat_ioctl = pngd_compat_ioctl,
#endif
};

hi_s32 pngd_init(void *p)
{
    PNGD_INFO_TRACE(" pngd_init start");

    if (g_pngd_state == PNGD_STATE_STARTED) {
        PNGD_INFO_TRACE("PNGD initialize again!");
        return HI_SUCCESS;
    }

    if (g_pngd_state == PNGD_STATE_STOPING) {
        PNGD_INFO_TRACE("PNGD is busy now!");
        return HI_ERR_PNGD_BUSY;
    }

    g_pngd_state = PNGD_STATE_STARTED;

    PNGD_INFO_TRACE(" pngd_init end");
    return HI_SUCCESS;
}

hi_void pngd_exit(void)
{
    if (g_pngd_state == PNGD_STATE_STOPED) {
        return;
    }
    // 通道销毁操作由fd关闭时, pngd_close保证

    g_pngd_state = PNGD_STATE_STOPED;

    return;
}

hi_void pngd_query_state(mod_state *state)
{
    if (osal_atomic_read(&g_pngd_user_ref) == 0) {
        *state = MOD_STATE_FREE;
    } else {
        *state = MOD_STATE_BUSY;
    }
    return;
}

hi_void pngd_notify(mod_notice_id notice)
{
    hi_u32 i;

    g_pngd_state = PNGD_STATE_STOPING;

    for (i = 0; i < PngdMaxChnNum; i++) {
        if (osal_down(&g_pngd_chn[i].pngd_sem) == HI_SUCCESS) {
            if (g_pngd_chn[i].chn_state == PNGD_CREATED) {
                osal_wakeup(&g_pngd_chn[i].wait_stream);
                osal_wakeup(&g_pngd_chn[i].wait_pic);
            }
            osal_up(&g_pngd_chn[i].pngd_sem);
        }
    }

    return;
}

hi_u32 pngd_get_ver_magic(hi_void)
{
    return VERSION_MAGIC;
}

hi_void pngd_module_ref_ctrl(module_ref_ctrl ref_ctrl)
{
    if (ref_ctrl == MOD_REF_GET) {
        try_module_get(THIS_MODULE);
    } else if (ref_ctrl == MOD_REF_PUT) {
        if (module_refcount(THIS_MODULE) > 0) {
            module_put(THIS_MODULE);
        }
    } else {
        PNGD_ERR_TRACE("unsupport operate %d\n", (hi_s32)ref_ctrl);
    }
}

hi_s32 pngd_freeze(osal_dev_t *pdev)
{
    return HI_SUCCESS;
}

hi_s32 pngd_restore(osal_dev_t *pdev)
{
    return HI_SUCCESS;
}

void pngd_add_chn(void)
{
    if (g_created_pngd_chn_num == 0) {
        PNGD_INFO_TRACE(" Create PNGD and lock module\n");
        try_module_get(THIS_MODULE);
    }
    g_created_pngd_chn_num++;
}

void pngd_del_chn(void)
{
    if (g_created_pngd_chn_num <= 0) {
        PNGD_ERR_TRACE(" Created PNGD chn num %d is abnormal, can't destroy pngd\n",
                       g_created_pngd_chn_num);
        return;
    }

    g_created_pngd_chn_num--;
    if (g_created_pngd_chn_num == 0) {
        PNGD_INFO_TRACE(" Destroy PNGD and unlock module\n");
        module_put(THIS_MODULE);
    }
}

static pngd_extern_func s_export_funcs = {
    .pfn_pngd_add_chn = pngd_add_chn,
    .pfn_pngd_del_chn = pngd_del_chn,
};

static umap_module s_pngd_module = {
    .mod_id = HI_ID_PNGD,
    .mod_name = "pngd",

    .pfn_init = pngd_init,
    .pfn_exit = pngd_exit,
    .pfn_bind = NULL,
    .pfn_ref_ctrl = pngd_module_ref_ctrl,
    .pfn_query_state = pngd_query_state,
    .pfn_notify = pngd_notify,
    .pfn_ver_checker = pngd_get_ver_magic,
    .export_funcs = &s_export_funcs,
    .data = HI_NULL,
};

struct osal_pmops pngd_drv_ops = {
    .pm_freeze = pngd_freeze,
    .pm_restore = pngd_restore,
};

hi_s32 pngd_all_ctx_init(hi_void)
{
    hi_u32 i = 0;
    hi_s32 ret = 0;

    g_pngd_state_spin_lock.lock = NULL;

    ret = memset_s(g_pngd_proc, sizeof(pngd_proc) * PngdMaxChnNum, 0, sizeof(pngd_proc) * PngdMaxChnNum);
    if (ret != 0) {
        PNGD_INFO_TRACE("memset_s failed! ret = %d\n", ret);
        return ret;
    }
    ret = memset_s(g_pngd_chn, sizeof(pngd_context) * PngdMaxChnNum, 0, sizeof(pngd_context) * PngdMaxChnNum);
    if (ret != 0) {
        PNGD_INFO_TRACE("memset_s failed! ret = %d\n", ret);
        return ret;
    }

    ret = osal_spin_lock_init(&g_pngd_state_spin_lock);
    if (ret != 0) {
        PNGD_INFO_TRACE("g_state_spin_lock init failed! ret = %d\n", ret);
        return ret;
    }

    for (i = 0; i < PngdMaxChnNum; i++) {
        /* init pngd channel mutex */
        ret = osal_sema_init(&g_pngd_chn[i].pngd_sem, 1);
        if (ret != 0) {
            PNGD_INFO_TRACE("g_pngd_chn[%d].pngd_sem init failed! ret = %d\n", i, ret);
            return ret;
        }
        ret = osal_spin_lock_init(&g_pngd_chn[i].stream_spin_lock);
        if (ret != 0) {
            PNGD_INFO_TRACE("g_pngd_chn[%d].stream_spin_lock init failed! ret = %d\n", i, ret);
            return ret;
        }

        /* init channel state:created=false,destroyed=true */
        g_pngd_chn[i].chn_id = i;
        g_pngd_chn[i].chn_state = PNGD_DESTROYED;
        g_pngd_chn[i].unique_id = 0;

        ret = osal_wait_init(&g_pngd_chn[i].wait_stream);
        if (ret != 0) {
            PNGD_INFO_TRACE("g_pngd_chn[%d].wait_stream init failed! ret = %d\n", i, ret);
            return ret;
        }
        ret = osal_wait_init(&g_pngd_chn[i].wait_pic);
        if (ret != 0) {
            PNGD_ERR_TRACE("g_pngd_chn[%d].wait_pic init failed! ret = %d\n", i, ret);
            return ret;
        }
    }

    ret = pngd_chn_vf_manager_init();
    if (ret != 0) {
        PNGD_ERR_TRACE("pngd_chn_vf_manager_init init failed! ret = %d\n", ret);
        return ret;
    }
    return HI_SUCCESS;
}

hi_void pngd_all_ctx_de_init(hi_void)
{
    hi_u32 i;
    pngd_chn_vf_manager_uninit();

    for (i = 0; i < PngdMaxChnNum; i++) {
        osal_sema_destory(&g_pngd_chn[i].pngd_sem);
        osal_spin_lock_destory(&g_pngd_chn[i].stream_spin_lock);

        osal_wait_destory(&g_pngd_chn[i].wait_pic);
        osal_wait_destory(&g_pngd_chn[i].wait_stream);
    }
    osal_spin_lock_destory(&g_pngd_state_spin_lock);

    return;
}

hi_s32 pngd_check_module_param(void)
{
    if (CHIP_SUPPORT_DEC() == 0) {
        PNGD_INFO_TRACE("Unsupport to load pngd module!\n");
        return HI_FAILURE;
    }

    if ((PngdMaxChnNum < 1) || (PngdMaxChnNum > INTERNAL_PNGD_MAX_CHN_NUM)) {
        PNGD_INFO_TRACE("Module_param: PngdMaxChnNum(%d) is illegal,should be [1, %d]\n",
            PngdMaxChnNum, INTERNAL_PNGD_MAX_CHN_NUM);
        PngdMaxChnNum = INTERNAL_PNGD_MAX_CHN_NUM;
    }

    return HI_SUCCESS;
}

hi_s32 pngd_alloc_ctx(void)
{
    hi_s32 ret = 0;
    hi_u32 total_ctx_size = 0;
    hi_u32 total_proc_size = 0;

    total_ctx_size = PngdMaxChnNum * sizeof(pngd_context);
    total_proc_size = PngdMaxChnNum * sizeof(pngd_proc);

    g_pngd_chn = (pngd_context *)osal_vmalloc(total_ctx_size + total_proc_size);
    if (g_pngd_chn == NULL) {
        PNGD_INFO_TRACE("malloc pngd context fail! total_ctx_size %d.\n", total_ctx_size);
        return HI_FAILURE;
    }
    ret = memset_s(g_pngd_chn, (total_ctx_size + total_proc_size),
                   0, (total_ctx_size + total_proc_size));
    if (ret != 0) {
        PNGD_INFO_TRACE("memset_s failed! ret=%d\n", ret);
        osal_vfree(g_pngd_chn);
        g_pngd_chn = HI_NULL;
        g_pngd_proc = HI_NULL;
        return HI_FAILURE;
    }

    g_pngd_proc = (pngd_proc *)((hi_uintptr_t)g_pngd_chn + total_ctx_size);
    return HI_SUCCESS;
}

hi_void pngd_free_ctx(void)
{
    if (g_pngd_chn != NULL) {
        osal_vfree(g_pngd_chn);
        g_pngd_chn = HI_NULL;
        g_pngd_proc = HI_NULL;
    }
}

hi_s32 pngd_register_to_sys(void)
{
    hi_s32 ret = HI_SUCCESS;
    if (pngd_check_module_param() != HI_SUCCESS) {
        PNGD_ERR_TRACE("load pngd.ko fail!\n");
        return HI_FAILURE;
    }

    if (pngd_alloc_ctx() != HI_SUCCESS) {
        pngd_free_ctx();
        PNGD_ERR_TRACE("load pngd.ko fail!\n");
        return HI_FAILURE;
    }

    g_pngd_umapd = osal_createdev(UMAP_DEVNAME_PNGD_BASE);
    if (g_pngd_umapd == NULL) {
        pngd_free_ctx();
        PNGD_ERR_TRACE("PNGD createdev failed! load pngd.ko failed\n");
        return HI_FAILURE;
    }
    g_pngd_umapd->fops = &pngd_fops;
    g_pngd_umapd->minor = UMAP_PNGD_MINOR_BASE;
    g_pngd_umapd->osal_pmops = &pngd_drv_ops;

    ret = osal_registerdevice(g_pngd_umapd);
    if (ret != 0) {
        PNGD_ERR_TRACE("pngd register failed!");
        osal_destroydev(g_pngd_umapd);
        g_pngd_umapd = HI_NULL;
        pngd_free_ctx();
    }

    return ret;
}

int32_t pngd_module_init(void)
{
    hi_s32 ret;
    osal_proc_entry_t *proc = NULL;
    dvpp_vf_calc *vf_calc = sys_get_dvpp_vf_calc();

    PNGD_INFO_TRACE("load pngd.ko ....Start\n");

    ret = pngd_register_to_sys();
    if (ret != HI_SUCCESS) {
        PNGD_ERR_TRACE("pngd.ko register to sys fail!\n");
        return HI_FAILURE;
    }

    proc = osal_create_proc_entry(PROC_ENTRY_PNGD, NULL);
    if (proc == NULL) {
        goto FAIL1;
    }
    proc->read = pngd_proc_show;

    ret = pngd_all_ctx_init();
    if (ret != 0) {
        PNGD_ERR_TRACE("pngd_all_ctx_init failed!");
        goto FAIL2;
    }

    if (cmpi_register_module(&s_pngd_module)) {
        PNGD_ERR_TRACE("cmpi_register_module pngd.ko failed.\n");
        goto FAIL2;
    }

    ret = osal_atomic_init(&g_pngd_user_ref);
    if (ret != 0) {
        PNGD_ERR_TRACE("osal_atomic_init failed, ret = %d\n", ret);
        goto FAIL3;
    }
    osal_atomic_set(&g_pngd_user_ref, 0);
    ret = osal_sema_init(&g_pngd_sem, 1);
    if (ret != 0) {
        PNGD_ERR_TRACE("osal_sema_init g_pngd_sem failed, ret = %d\n", ret);
        osal_atomic_destory(&g_pngd_user_ref);
        goto FAIL3;
    }
    call_dvpp_comm_export_func_lock();
    vf_calc->dvpp_pngd_calc_init = pngd_calc_init;
    vf_calc->dvpp_pngd_calc_uninit = pngd_calc_uninit;
    call_dvpp_comm_export_func_unlock();

    PNGD_INFO_TRACE("load pngd.ko ....OK\n");
    return HI_SUCCESS;

FAIL3:
    cmpi_unregister_module(HI_ID_PNGD);

FAIL2:
    pngd_all_ctx_de_init();
    osal_remove_proc_entry(PROC_ENTRY_PNGD, NULL);

FAIL1:
    osal_deregisterdevice(g_pngd_umapd);

    osal_destroydev(g_pngd_umapd);
    g_pngd_umapd = HI_NULL;
    pngd_free_ctx();
    PNGD_ERR_TRACE("load pngd.ko fail!\n");
    return HI_FAILURE;
}

void pngd_module_exit(void)
{
    dvpp_vf_calc *vf_calc = sys_get_dvpp_vf_calc();
    if (g_pngd_chn == HI_NULL) {
        return;
    }

    osal_atomic_destory(&g_pngd_user_ref);

    cmpi_unregister_module(HI_ID_PNGD);

    pngd_all_ctx_de_init();
    osal_remove_proc_entry(PROC_ENTRY_PNGD, NULL);
    osal_deregisterdevice(g_pngd_umapd);
    osal_destroydev(g_pngd_umapd);
    osal_sema_destory(&g_pngd_sem);

    pngd_free_ctx();

    call_dvpp_comm_export_func_lock();
    vf_calc->dvpp_pngd_calc_init = NULL;
    vf_calc->dvpp_pngd_calc_uninit = NULL;
    call_dvpp_comm_export_func_unlock();

    PNGD_INFO_TRACE("unload pngd.ko ....OK\n");
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif // end of #ifdef __cplusplus
