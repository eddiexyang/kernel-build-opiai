/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: tde osr
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */

#ifndef __RTOS__
#include <linux/version.h>
#include "ot_tde_mod_init.h"
#ifdef CONFIG_64BIT
#include <asm/compat.h>
#endif
#endif
#include <linux/ascend_smmu.h> /* used by svm_get_pasid */
#include "tde_ioctl.h" /* because of redefined warning, tde_ioctl.h not go first. */
#include "ot_drv_tde.h"
#include "tde_osictl.h"
#include "tde_osilist.h"
#include "tde_hal.h"
#include "tde_handle.h"
#include "tde_adp.h"
#include "ot_common.h"
#include "dev_ext.h"
#include "wmalloc.h"
#include "securec.h"
#include "ot_osal.h"
#include "vdp_comm_pid_check.h"
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
#include "tde_proc.h"
#endif

static osal_spinlock_t g_tasklet_lock;

static osal_spinlock_t g_tde_ref_lock;

pid_spin_lock_s g_pid_lock;

typedef td_s32 (*tde_ctl_ptr_func)(unsigned long argp, td_void *private_data);
typedef struct {
    td_u32 tde_cmd;
    tde_ctl_ptr_func tde_ctl_ptr_func;
} tde_ctl_func_dispatch_item;

static td_s32 tde_osr_begin_job(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_bit_blit(unsigned long argp,  td_void *const private_data);
static td_s32 tde_osr_solid_draw(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_quick_copy(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_quick_resize(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_quick_fill(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_quick_draw(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_multi_draw(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_draw_line(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_quick_deflicker(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_mb_bit_blit(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_end_job(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_wait_for_done(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_cancel_job(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_bit_map_mask_rop(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_bit_map_mask_blend(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_wait_all_done(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_reset(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_set_deflicker_level(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_get_deflicker_level(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_set_alpha_threshold_value(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_get_alpha_threshold_value(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_set_alpha_threshold_state(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_get_alpha_threshold_state(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_pattern_fill(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_enable_region_deflicker(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_rotate(unsigned long argp, td_void *const private_data);
static td_s32 tde_osr_bind_smmu(td_void);
static td_s32 tde_osr_map_mempool(td_void);
static td_void tde_osr_unmap_mempool(td_void);

tde_ctl_func_dispatch_item g_tde_ctl_func_dispatch_item[] = {
    {TDE_BEGIN_JOB, tde_osr_begin_job},
    {TDE_END_JOB, tde_osr_end_job},
    {TDE_WAITFORDONE, tde_osr_wait_for_done},
    {TDE_WAITALLDONE, tde_osr_wait_all_done},
    {TDE_CANCEL_JOB, tde_osr_cancel_job},
    {TDE_QUICK_COPY, tde_osr_quick_copy},
    {TDE_QUICK_FILL, tde_osr_quick_fill},
    {TDE_PATTERN_FILL, tde_osr_pattern_fill},
#ifdef TDE_IOCTL_RESERVE
    {TDE_BIT_BLIT, tde_osr_bit_blit},
    {TDE_SOLID_DRAW, tde_osr_solid_draw},
    {TDE_QUICK_RESIZE, tde_osr_quick_resize},
    {TDE_QUICK_DEFLICKER, tde_osr_quick_deflicker},
    {TDE_MB_BITBLT, tde_osr_mb_bit_blit},
    {TDE_BITMAP_MASKROP, tde_osr_bit_map_mask_rop},
    {TDE_BITMAP_MASKBLEND, tde_osr_bit_map_mask_blend},
    {TDE_RESET, tde_osr_reset},
    {TDE_SET_DEFLICKERLEVEL, tde_osr_set_deflicker_level},
    {TDE_GET_DEFLICKERLEVEL, tde_osr_get_deflicker_level},
    {TDE_SET_ALPHATHRESHOLD_VALUE, tde_osr_set_alpha_threshold_value},
    {TDE_GET_ALPHATHRESHOLD_VALUE, tde_osr_get_alpha_threshold_value},
    {TDE_SET_ALPHATHRESHOLD_STATE, tde_osr_set_alpha_threshold_state},
    {TDE_GET_ALPHATHRESHOLD_STATE, tde_osr_get_alpha_threshold_state},
    {TDE_ENABLE_REGIONDEFLICKER, tde_osr_enable_region_deflicker},
    {TDE_ROTATE, tde_osr_rotate},
    {TDE_QUICK_DRAW_RECT, tde_osr_quick_draw},
    {TDE_DRAW_MULTI_RECT, tde_osr_multi_draw},
    {TDE_DRAW_LINE, tde_osr_draw_line}
#endif
};

#define TDE_IOCTL_FUNC_NUM (sizeof(g_tde_ctl_func_dispatch_item) / sizeof(g_tde_ctl_func_dispatch_item[0]))

static int tde_osr_isr(int irq, td_void *const dev_id);
static void tde_tasklet_func(unsigned long int_status);

/* TDE equipment quoted count */
typedef struct {
    int32_t tde_count;
    int32_t tgid;
    osal_mutex_t lock;
} tde_proc_info;

static tde_proc_info g_tde_info = { 0 };
static osal_atomic_t g_hifb_use_count = OSAL_ATOMIC_INIT(0);
#ifdef TDE_TIME_COUNT
tde_timeval g_time_end;
#endif

struct osal_fileops g_tde_fops = {
    .module= THIS_MODULE,
    .open = tde_open,
    .release = tde_release,
#ifdef CONFIG_COMPAT
    .compat_ioctl = tde_ioctl, /* tde_compactioctl */
#endif
    .unlocked_ioctl = tde_ioctl,
};

unsigned int g_tde_irq = TDE_INTNUM;
static osal_dev_t *g_tde_device;
#ifndef __RTOS__
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)
DECLARE_TASKLET_OLD(tde_tasklet, tde_tasklet_func);
#else
DECLARE_TASKLET(tde_tasklet, tde_tasklet_func, 0);
#endif
#endif

#ifdef CONFIG_TDE_TDE_EXPORT_FUNC
static ot_tde_export_func g_tde_export_func = {
    .drv_tde_module_open = tde_osi_open,
    .drv_tde_module_close = tde_osi_close,
    .drv_tde_module_begin_job = tde_osi_begin_job_ex,
    .drv_tde_module_end_job = tde_osi_end_job,
    .drv_tde_module_cancel_job = tde_osi_cancel_job,
    .drv_tde_module_wait_for_done = tde_osi_wait_for_done,
    .drv_tde_module_wait_all_done = tde_osi_wait_all_done,
    .drv_tde_module_quick_copy = tde_osi_quick_copy,
    .drv_tde_module_quick_fill = tde_osi_quick_fill,
    .drv_tde_module_quick_resize = tde_osi_quick_resize,
#if (TDE_CAPABILITY & DEFLICKER)
    .drv_tde_module_quick_flicker = tde_osi_quick_flicker,
#endif
    .drv_tde_module_blit = tde_osi_blit,
    .drv_tde_module_mb_blit = tde_osi_mb_blit,
    .drv_tde_module_solid_draw = tde_osi_solid_draw,
#if (TDE_CAPABILITY & DEFLICKER)
    .drv_tde_module_set_deflicker_level = tde_osi_set_deflicker_level,
#endif
    .drv_tde_module_enable_region_deflicker = tde_osi_enable_region_deflicker,
#if (TDE_CAPABILITY & ROTATE)
    .drv_tde_module_rotate = tde_osi_quick_rotate,
#endif
    .drv_tde_module_set_alpha_threshold_value = tde_osi_set_alpha_threshold_value,
    .drv_tde_module_set_alpha_threshold_state = tde_osi_set_alpha_threshold_state,
    .drv_tde_module_get_alpha_threshold_state = tde_osi_get_alpha_threshold_state,
    .drv_tde_module_cal_scale_rect = tde_cal_scale_rect,

#ifdef CONFIG_TDE_BLIT_EX
    .drv_tde_module_single_blit_ex = tde_osi_single_blit_ex,
#endif
};
#endif

td_void set_tde_irq(unsigned int temp_tde_irq)
{
    g_tde_irq = temp_tde_irq;
}

static td_s32 gfx_init(td_void *const args)
{
    ot_unused(args);
    return 0;
}

static td_void gfx_exit(td_void)
{
    return;
}

static td_void gfx_notify(mod_notice_id notice)
{
    ot_unused(notice);
    return;
}

static td_void gfx_query_state(mod_state *state)
{
    if (state == TD_NULL) {
        return;
    }
    *state = MOD_STATE_FREE;
    return;
}

static td_u32 gfx_get_ver_magic(td_void)
{
    return VERSION_MAGIC;
}

static umap_module g_module = {
    .mod_id = OT_ID_TDE,
    .mod_name = "tde",
    .pfn_init = gfx_init,
    .pfn_exit = gfx_exit,
    .pfn_query_state = gfx_query_state,
    .pfn_notify = gfx_notify,
    .pfn_ver_checker = gfx_get_ver_magic,
    .export_funcs = &g_tde_export_func,
    .data = TD_NULL,
    };

static td_s32 tde_init_lock_and_count_res(td_void)
{
    int ret;
    do {
        ret = osal_spin_lock_init(&g_pid_lock);
        if (ret != TD_SUCCESS) {
            break;
        }
        ret = osal_spin_lock_init(&g_tasklet_lock);
        if (ret != TD_SUCCESS) {
            osal_spin_lock_destroy(&g_pid_lock);
            break;
        }
        ret = osal_spin_lock_init(&g_tde_ref_lock);
        if (ret != TD_SUCCESS) {
            osal_spin_lock_destroy(&g_tasklet_lock);
            osal_spin_lock_destroy(&g_pid_lock);
            break;
        }
        (void)memset_s(&g_tde_info, sizeof(tde_proc_info), 0x0, sizeof(tde_proc_info));
        ret = osal_mutex_init(&g_tde_info.lock);
        if (ret != TD_SUCCESS) {
            osal_spin_lock_destroy(&g_tde_ref_lock);
            osal_spin_lock_destroy(&g_tasklet_lock);
            osal_spin_lock_destroy(&g_pid_lock);
            break;
        }
        ret = osal_atomic_init(&g_hifb_use_count);
        if (ret != TD_SUCCESS) {
            osal_mutex_destroy(&g_tde_info.lock);
            osal_spin_lock_destroy(&g_tde_ref_lock);
            osal_spin_lock_destroy(&g_tasklet_lock);
            osal_spin_lock_destroy(&g_pid_lock);
            break;
        }
        (void)osal_atomic_set(&g_hifb_use_count, 0);
    } while (0);
    if (ret != TD_SUCCESS) {
        tde_error("tde init common res failed. ret %d\n", ret);
    }
    return ret;
}

static td_void tde_destroy_lock_and_count_res(td_void)
{
    osal_spin_lock_destroy(&g_tasklet_lock);
    osal_spin_lock_destroy(&g_tde_ref_lock);
    osal_mutex_destroy(&g_tde_info.lock);
    osal_atomic_destroy(&g_hifb_use_count);
    osal_spin_lock_destroy(&g_pid_lock);
}

static td_s32 tde_init_create(td_void)
{
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_proc_entry_t *proc = TD_NULL;

    proc = osal_create_proc_entry("tde", TD_NULL);
    if (proc == TD_NULL) {
        tde_error("TDE module create proc failure!\n");
        tde_destroy_lock_and_count_res();
        osal_deregisterdevice(g_tde_device);
        (void)osal_destroydev(g_tde_device);
        tde_osi_list_term();
        osal_free_irq(g_tde_irq, tde_osr_isr);
        return TD_FAILURE;
    }
    proc->read = tde_read_proc;
#endif
    if (cmpi_register_module(&g_module)) {
        tde_error("cmpi_register_module failed\n");
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
        osal_remove_proc_entry("tde", TD_NULL);
        proc = NULL;
#endif
        tde_destroy_lock_and_count_res();
        osal_deregisterdevice(g_tde_device);
        (void)osal_destroydev(g_tde_device);
        tde_osi_list_term();
        osal_free_irq(g_tde_irq, tde_osr_isr);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

td_s32 tde_init_module_k(td_void)
{
    if (tde_hal_init(TDE_REG_BASEADDR) < 0) {
        tde_error("tde_hal_init failure!\n");
        return -1;
    }

    if (osal_request_irq(g_tde_irq, tde_osr_isr, TD_NULL, TDE_IRQ_NAME, tde_osr_isr) != 0) {
        tde_error("request_irq for TDE failure!\n");
        tde_hal_release();
        return -1;
    }

    if (tde_osi_list_init() != 0) {
        tde_error("tde_osi_list_init failure!\n");
        osal_free_irq(g_tde_irq, tde_osr_isr);
        tde_hal_release();
        return -1;
    }

    /* register tde device */
    g_tde_device = osal_createdev("ot_tde");
    if (g_tde_device == TD_NULL) {
        tde_error("\n  osal_createdev failure\n");
        tde_osi_list_term();
        osal_free_irq(g_tde_irq, tde_osr_isr);
        tde_hal_release();
        return -1;
    }
    g_tde_device->minor = UMAP_TDE_MINOR_BASE; /* 255 max bits */
    g_tde_device->fops = &g_tde_fops;
    if (osal_registerdevice(g_tde_device) != TD_SUCCESS) {
        tde_error("\n  osal_registerdevice failure\n");
        (void)osal_destroydev(g_tde_device);
        tde_osi_list_term();
        osal_free_irq(g_tde_irq, tde_osr_isr);
        tde_hal_release();
        return TD_FAILURE;
    }
    if (tde_init_lock_and_count_res() != TD_SUCCESS) {
        tde_error("\n  tde init lock and count res failure\n");
        osal_deregisterdevice(g_tde_device);
        (void)osal_destroydev(g_tde_device);
        tde_osi_list_term();
        osal_free_irq(g_tde_irq, tde_osr_isr);
        tde_hal_release();
        return TD_FAILURE;
    }
    if (tde_init_create() != TD_SUCCESS) {
        tde_error("\n  tde init and create failure\n");
        tde_destroy_lock_and_count_res();
        osal_deregisterdevice(g_tde_device);
        (void)osal_destroydev(g_tde_device);
        tde_osi_list_term();
        osal_free_irq(g_tde_irq, tde_osr_isr);
        tde_hal_release();
        return TD_FAILURE;
    }
    return 0;
}

td_void tde_cleanup_module_k(td_void)
{
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    osal_remove_proc_entry("tde", TD_NULL);
#endif
    tde_osi_list_term();
    osal_free_irq(g_tde_irq, tde_osr_isr);

    cmpi_unregister_module((hi_mod_id)(conver_id(OT_GFX_TDE_ID)));

    tde_destroy_lock_and_count_res();
    /* cleanup_module is never called if registering failed */
    osal_deregisterdevice(g_tde_device);
    (void)osal_destroydev(g_tde_device);
    tde_hal_release();
    return;
}

static hi_bool ioctl_check_tgid(void)
{
    unsigned long lockflag;
    pid_spin_lock_irqsave(&g_pid_lock, lockflag);
    if (g_tde_info.tgid == current->tgid) {
        pid_spin_unlock_irqrestore(&g_pid_lock, lockflag);
        return TD_TRUE;
    }
    pid_spin_unlock_irqrestore(&g_pid_lock, lockflag);
    return TD_FALSE;
}

int tde_open(td_void* private_data)
{
    ot_unused(private_data);
    osal_mutex_lock(&g_tde_info.lock);
    int ret = 0;
    do {
        if (g_tde_info.tde_count < 0) {
            tde_error("tde info count err ! tde count %d \n", g_tde_info.tde_count);
            ret = -1;
            break;
        } else if (g_tde_info.tde_count > 0) {
            if (g_tde_info.tgid != current->tgid) {
                tde_error("tde only support process tgid %d open ! \n", g_tde_info.tgid);
                ret = -1;
                break;
            }
        } else {
#ifdef ENV_SMMU_SUPPORT
            if (tde_osr_bind_smmu() != 0) {
                tde_error("tde bind smmu failed ! \n");
                ret = -1;
                break;
            }
            if (tde_osr_map_mempool() != 0) {
                tde_error("tde set mempool failed ! \n");
                ret = -1;
                break;
            }
            if (tde_ptable_open() != 0) {
                tde_osr_unmap_mempool();
                tde_error("tde ptable open failed ! \n");
                ret = -1;
                break;
            }
#endif
            g_tde_info.tgid = current->tgid;
#ifndef TDE_LP_MODE
            tde_hal_set_clock(TD_TRUE);
#endif
        }
        g_tde_info.tde_count++;
    } while (0);
    osal_mutex_unlock(&g_tde_info.lock);
    return ret;
}

int tde_release(td_void *private_data)
{
    int ret = 0;
    if (private_data != TD_NULL) {
        tde_list_free_pending_job(private_data);
    }
    osal_mutex_lock(&g_tde_info.lock);
    if (g_tde_info.tde_count == 1) {
#ifndef TDE_LP_MODE
        tde_hal_set_clock(TD_FALSE);
#endif
#ifdef ENV_SMMU_SUPPORT
        tde_osr_unmap_mempool();
        tde_ptable_close();
#endif
        g_tde_info.tgid = 0;
    } else if (g_tde_info.tde_count <= 0) {
        tde_error("tde info count err! tde count %d", g_tde_info.tde_count);
        ret = -1;
        osal_mutex_unlock(&g_tde_info.lock);
        return ret;
    }
    g_tde_info.tde_count--;
    osal_mutex_unlock(&g_tde_info.lock);
    return ret;
}

long tde_ioctl(unsigned int cmd, unsigned long arg, td_void *private_data)
{
    td_u32 i;
    if (ioctl_check_tgid() != TD_TRUE) {
        tde_info("the ioctl cmd if not from the same pid \n");
        return DRV_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    for (i = 0; i < TDE_IOCTL_FUNC_NUM; i++) {
        if (cmd == g_tde_ctl_func_dispatch_item[i].tde_cmd) {
            break;
        }
    }

    if (i == TDE_IOCTL_FUNC_NUM) {
        tde_error("cmd 0x%x is invalid!", cmd);
        return -EFAULT;
    }

    if (g_tde_ctl_func_dispatch_item[i].tde_ctl_ptr_func == TD_NULL) {
        tde_error("tde_ctl_ptr_func is null!");
        return -EFAULT;
    }

    return g_tde_ctl_func_dispatch_item[i].tde_ctl_ptr_func(arg, private_data);
}

static td_s32 tde_osr_begin_job(unsigned long argp, td_void *private_data)
{
    td_s32 ret;
    td_s32 *handle = TD_NULL;

    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    handle = (td_s32 *)(uintptr_t)argp;
    ret = tde_osi_begin_job(handle, private_data);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return 0;
}

static td_s32 tde_osr_bit_blit(unsigned long argp, td_void * const private_data)
{
    drv_tde_double_src double_src;
    drv_tde_opt *opt = TD_NULL;
    drv_tde_bitblit_cmd *bit_blt = TD_NULL;
    ot_unused(private_data);
    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    bit_blt = (drv_tde_bitblit_cmd *)(uintptr_t)argp;

    double_src.bg_surface = ((bit_blt->null_indicator >> 1) & 1) ? TD_NULL : &(bit_blt->bg_surface);
    /* 2 Take the high */
    double_src.bg_rect = ((bit_blt->null_indicator >> 2) & 1) ? TD_NULL : &(bit_blt->bg_rect);
    /* 3 Take the high */
    double_src.fg_surface = ((bit_blt->null_indicator >> 3) & 1) ? TD_NULL : &(bit_blt->fg_surface);
	/* 4 Take the high */
    double_src.fg_rect = ((bit_blt->null_indicator >> 4) & 1) ? TD_NULL : &(bit_blt->fg_rect);
    double_src.dst_surface = &(bit_blt->dst_surface);
    double_src.dst_rect = &(bit_blt->dst_rect);
    opt = ((bit_blt->null_indicator >> 7) & 1) ? TD_NULL : &(bit_blt->option); /* 7 Take the high */
    return tde_osi_blit(bit_blt->handle, &double_src, opt);
}

static td_s32 tde_osr_solid_draw(unsigned long argp,  td_void *const private_data)
{
    drv_tde_fill_color *fill_color = TD_NULL;
    drv_tde_opt *opt = TD_NULL;
    drv_tde_solid_draw_cmd *solid_draw = TD_NULL;
    drv_tde_single_src single_src;
    ot_unused(private_data);
    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    solid_draw = (drv_tde_solid_draw_cmd *)(uintptr_t)argp;

    single_src.src_surface = ((solid_draw->null_indicator >> 1) & 1) ? TD_NULL : &(solid_draw->fg_surface);
    /* 2 Take the high */
    single_src.src_rect = ((solid_draw->null_indicator >> 2) & 1) ? TD_NULL : &(solid_draw->fg_rect);
    single_src.dst_surface = &(solid_draw->dst_surface);
    single_src.dst_rect = &(solid_draw->dst_rect);
    /* 5 Take the high */
    fill_color = ((solid_draw->null_indicator >> 5) & 1) ? TD_NULL : &(solid_draw->fill_color);
    opt = ((solid_draw->null_indicator >> 6) & 1) ? TD_NULL : &(solid_draw->option); /* 6 Take the high */
    return tde_osi_solid_draw(solid_draw->handle, &single_src, fill_color, opt);
}

static td_s32 tde_osr_quick_copy(unsigned long argp, td_void *const private_data)
{
    drv_tde_quick_copy_cmd *quick_copy = TD_NULL;
    drv_tde_single_src single_src;
    ot_unused(private_data);
    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    quick_copy = (drv_tde_quick_copy_cmd *)(uintptr_t)argp;

    single_src.src_surface = &(quick_copy->src_surface);
    single_src.src_rect = &(quick_copy->src_rect);
    single_src.dst_surface = &(quick_copy->dst_surface);
    single_src.dst_rect = &(quick_copy->dst_rect);

    return tde_osi_quick_copy(quick_copy->handle, &single_src);
}

static td_s32 tde_osr_quick_resize(unsigned long argp, td_void *const private_data)
{
    drv_tde_quick_resize_cmd *quick_resize = TD_NULL;
    ot_unused(private_data);
    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    quick_resize = (drv_tde_quick_resize_cmd *)(uintptr_t)argp;

    return tde_osi_quick_resize(quick_resize->handle, &(quick_resize->src_surface), &(quick_resize->src_rect),
                                &(quick_resize->dst_surface), &(quick_resize->dst_rect));
}
static td_s32 tde_osr_quick_fill(unsigned long argp, td_void *const private_data)
{
    drv_tde_quick_fill_cmd *quick_fill = TD_NULL;
    ot_unused(private_data);
    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    quick_fill = (drv_tde_quick_fill_cmd *)(uintptr_t)argp;

    return tde_osi_quick_fill(quick_fill->handle, &(quick_fill->dst_surface), &(quick_fill->dst_rect),
                              quick_fill->fill_data);
}

static td_s32 tde_osr_quick_draw(unsigned long argp, td_void *const private_data)
{
    drv_tde_quick_draw_cmd *quick_draw = TD_NULL;
    ot_unused(private_data);
    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    quick_draw = (drv_tde_quick_draw_cmd *)(uintptr_t)argp;

    return tde_osi_quick_draw(quick_draw->handle, &(quick_draw->dst_surface), &(quick_draw->dst_rect),
                              &(quick_draw->corner_rect_info));
}

static td_s32 tde_osr_multi_draw(unsigned long argp, td_void *const private_data)
{
    td_s32 ret;
    drv_tde_multi_draw_cmd *multi_draw = TD_NULL;
    drv_tde_rect *corner_rect_region = TD_NULL;
    drv_tde_corner_rect_info *corner_rect_list = TD_NULL;
    ot_unused(private_data);
    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    multi_draw = (drv_tde_multi_draw_cmd *)(uintptr_t)argp;

    if (multi_draw->corner_rect_num == 0 || multi_draw->corner_rect_num > 128) { /* 128 max */
        tde_error("num is invalid,num = %u!\n", multi_draw->corner_rect_num);
        return DRV_ERR_TDE_INVALID_PARA;
    }
    corner_rect_region = (drv_tde_rect *)osal_vmalloc(sizeof(drv_tde_rect) * multi_draw->corner_rect_num);
    if (corner_rect_region == TD_NULL) {
        tde_error("osal_vmalloc failed!\n");
        return DRV_ERR_TDE_NO_MEM;
    }
    ret = (td_s32)osal_copy_from_user(corner_rect_region, (td_void *)multi_draw->corner_rect_region,
        sizeof(drv_tde_rect) * multi_draw->corner_rect_num);
    if (ret != TD_SUCCESS) {
        tde_error("copy corner_rect_region failed!\n");
        osal_vfree(corner_rect_region);
        corner_rect_region = TD_NULL;
        return DRV_ERR_TDE_NO_MEM;
    }
    corner_rect_list = (drv_tde_corner_rect_info *)osal_vmalloc(sizeof(drv_tde_corner_rect_info) *
        multi_draw->corner_rect_num);
    if (corner_rect_list == TD_NULL) {
        tde_error("osal_vmalloc failed!\n");
        osal_vfree(corner_rect_region);
        corner_rect_region = TD_NULL;
        return DRV_ERR_TDE_NO_MEM;
    }
    ret = (td_s32)osal_copy_from_user(corner_rect_list, (td_void *)multi_draw->corner_rect_info,
        sizeof(drv_tde_corner_rect_info) * multi_draw->corner_rect_num);
    if (ret != TD_SUCCESS) {
        tde_error("copy corner_rect_info failed!\n");
        osal_vfree(corner_rect_region);
        osal_vfree(corner_rect_list);
        corner_rect_region = TD_NULL;
        corner_rect_list = TD_NULL;
        return DRV_ERR_TDE_NO_MEM;
    }

    ret = tde_osi_multi_draw(multi_draw->handle, &(multi_draw->dst_surface), corner_rect_region, corner_rect_list,
        multi_draw->corner_rect_num);

    osal_vfree(corner_rect_region);
    osal_vfree(corner_rect_list);
    corner_rect_region = TD_NULL;
    corner_rect_list = TD_NULL;
    return ret;
}

static td_s32 tde_osr_draw_line(unsigned long argp, td_void *const private_data)
{
    td_s32 ret;
    drv_tde_draw_line_cmd *line_cmd = TD_NULL;
    drv_tde_line *line = TD_NULL;
    ot_unused(private_data);

    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    line_cmd = (drv_tde_draw_line_cmd *)(uintptr_t)argp;
    if ((line_cmd->line_num == 0) || (line_cmd->line_num > 100)) { /* 100 max */
        tde_error("num is invalid,num = %u!\n", line_cmd->line_num);
        return DRV_ERR_TDE_INVALID_PARA;
    }

    line = (drv_tde_line *)osal_vmalloc(sizeof(drv_tde_line) * line_cmd->line_num);
    if (line == TD_NULL) {
        tde_error("osal_vmalloc failed!\n");
        return DRV_ERR_TDE_NO_MEM;
    }

    ret = (td_s32)osal_copy_from_user(line, (td_void *)line_cmd->line, sizeof(drv_tde_line) * line_cmd->line_num);
    if (ret != TD_SUCCESS) {
        tde_error("copy line from user failed!\n");
        osal_vfree(line);
        line=TD_NULL;
        return DRV_ERR_TDE_NO_MEM;
    }

    ret = tde_osi_draw_line(line_cmd->handle, &(line_cmd->dst_surface), line, line_cmd->line_num);
    osal_vfree(line);
    line = TD_NULL;
    return ret;
}

static td_s32 tde_osr_quick_deflicker(unsigned long argp, td_void *const private_data)
{
#if (TDE_CAPABILITY & DEFLICKER)
    drv_tde_quick_deflicker_cmd *deflicker = TD_NULL;
    ot_unused(private_data);
    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    deflicker = (drv_tde_quick_deflicker_cmd *)(uintptr_t)argp;

    return tde_osi_quick_flicker(deflicker->handle, &(deflicker->src_surface), &(deflicker->src_rect),
                                 &(deflicker->dst_surface), &(deflicker->dst_rect));
#else
    ot_unused(argp);
    ot_unused(private_data);
    return DRV_ERR_TDE_UNSUPPORTED_OPERATION;
#endif
}

static td_s32 tde_osr_mb_bit_blit(unsigned long argp, td_void *const private_data)
{
    drv_tde_mb_blit_cmd *mb_bitblt = TD_NULL;
    drv_tde_mb_src mb_src;
    ot_unused(private_data);

    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    mb_bitblt = (drv_tde_mb_blit_cmd *)(uintptr_t)argp;

    mb_src.mb_surface = &(mb_bitblt->mb_src_surface);
    mb_src.src_rect = &(mb_bitblt->mb_src_rect);
    mb_src.dst_surface = &(mb_bitblt->dst_surface);
    mb_src.dst_rect = &(mb_bitblt->dst_rect);

    return tde_osi_mb_blit(mb_bitblt->handle, &mb_src, &(mb_bitblt->mb_option));
}

static td_s32 tde_osr_end_job(unsigned long argp, td_void *const private_data)
{
    drv_tde_end_job_cmd *end_job = TD_NULL;
    ot_unused(private_data);

    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    end_job = (drv_tde_end_job_cmd *)(uintptr_t)argp;

    return tde_osi_end_job(end_job, TD_NULL, TD_NULL);
}

static td_s32 tde_osr_wait_for_done(unsigned long argp, td_void *const private_data)
{
    td_s32 *handle = TD_NULL;
    ot_unused(private_data);

    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    handle = (td_s32 *)(uintptr_t)argp;

    return tde_osi_wait_for_done(*handle, TDE_MAX_WAIT_TIMEOUT);
}

static td_s32 tde_osr_cancel_job(unsigned long argp, td_void *const private_data)
{
    td_s32 *handle = TD_NULL;
    ot_unused(private_data);

    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    handle = (td_s32 *)(uintptr_t)argp;

    return tde_osi_cancel_job(*handle);
}

static td_s32 tde_osr_bit_map_mask_rop(unsigned long argp, td_void *const private_data)
{
#if (TDE_CAPABILITY & MASKROP)
    drv_tde_bitmap_maskrop_cmd *bmp_mask_rop = TD_NULL;
    drv_tde_triple_src triple_src = {0};
    ot_unused(private_data);

    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    bmp_mask_rop = (drv_tde_bitmap_maskrop_cmd *)(uintptr_t)argp;

    triple_src.bg_surface = &(bmp_mask_rop->bg_surface);
    triple_src.bg_rect = &(bmp_mask_rop->bg_rect);
    triple_src.fg_surface = &(bmp_mask_rop->fg_surface);
    triple_src.fg_rect = &(bmp_mask_rop->fg_rect);
    triple_src.mask_surface = &(bmp_mask_rop->mask_surface);
    triple_src.mask_rect = &(bmp_mask_rop->mask_rect);
    triple_src.dst_surface = &(bmp_mask_rop->dst_surface);
    triple_src.dst_rect = &(bmp_mask_rop->dst_rect);

    return tde_osi_bitmap_mask_rop(bmp_mask_rop->handle, &triple_src, bmp_mask_rop->rop_color, bmp_mask_rop->rop_alpha);
#else
    ot_unused(argp);
    ot_unused(private_data);
    return DRV_ERR_TDE_UNSUPPORTED_OPERATION;
#endif
}

static td_s32 tde_osr_bit_map_mask_blend(unsigned long argp, td_void *const private_data)
{
#if (TDE_CAPABILITY & MASKROP)
    drv_tde_bitmap_maskblend_cmd *bmp_mask_blend = TD_NULL;
    drv_tde_triple_src triple_src = {0};
    ot_unused(private_data);

    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    bmp_mask_blend = (drv_tde_bitmap_maskblend_cmd *)(uintptr_t)argp;

    triple_src.bg_surface = &(bmp_mask_blend->bg_surface);
    triple_src.bg_rect = &(bmp_mask_blend->bg_rect);
    triple_src.fg_surface = &(bmp_mask_blend->fg_surface);
    triple_src.fg_rect = &(bmp_mask_blend->fg_rect);
    triple_src.mask_surface = &(bmp_mask_blend->mask_surface);
    triple_src.mask_rect = &(bmp_mask_blend->mask_rect);
    triple_src.dst_surface = &(bmp_mask_blend->dst_surface);
    triple_src.dst_rect = &(bmp_mask_blend->dst_rect);

    return tde_osi_bitmap_mask_blend(bmp_mask_blend->handle, &triple_src, bmp_mask_blend->alpha,
                                     bmp_mask_blend->blend_mode);
#else
    ot_unused(argp);
    ot_unused(private_data);
    return DRV_ERR_TDE_UNSUPPORTED_OPERATION;
#endif
}

static td_s32 tde_osr_wait_all_done(unsigned long argp, td_void *const private_data)
{
    ot_unused(argp);
    ot_unused(private_data);
    return tde_osi_wait_all_done(TD_FALSE);
}

static td_s32 tde_osr_reset(unsigned long argp, td_void *const private_data)
{
    ot_unused(argp);
    ot_unused(private_data);
    return TD_SUCCESS;
}

static td_s32 tde_osr_set_deflicker_level(unsigned long argp, td_void *const private_data)
{
#if (TDE_CAPABILITY & DEFLICKER)
    drv_tde_deflicker_level deflicker_level;
    ot_unused(private_data);

    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    deflicker_level = *(drv_tde_deflicker_level *)(uintptr_t)argp;
    if (deflicker_level < 0 || deflicker_level >= DRV_TDE_DEFLICKER_LEVEL_MAX) {
        return DRV_ERR_TDE_INVALID_PARA;
    }
    return tde_osi_set_deflicker_level(deflicker_level);
#else
    ot_unused(argp);
    ot_unused(private_data);
    return DRV_ERR_TDE_UNSUPPORTED_OPERATION;
#endif
}

static td_s32 tde_osr_get_deflicker_level(unsigned long argp, td_void *private_data)
{
    drv_tde_deflicker_level *deflicker_level = TD_NULL;
    ot_unused(private_data);
    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    deflicker_level = (drv_tde_deflicker_level *)(uintptr_t)argp;
#if (TDE_CAPABILITY & DEFLICKER)
    if (tde_osi_get_deflicker_level(deflicker_level) != TD_SUCCESS) {
        return DRV_ERR_TDE_INVALID_PARA;
    }
#else
    *deflicker_level = DRV_TDE_DEFLICKER_LEVEL_AUTO;
#endif
    return TD_SUCCESS;
}

static td_s32 tde_osr_set_alpha_threshold_value(unsigned long argp, td_void *const private_data)
{
    td_u8 alpha_threshold_value;
    ot_unused(private_data);
    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    alpha_threshold_value = *(td_u8 *)(uintptr_t)argp;

    return tde_osi_set_alpha_threshold_value(alpha_threshold_value);
}

static td_s32 tde_osr_get_alpha_threshold_value(unsigned long argp, td_void *const private_data)
{
    td_u8 *alpha_threshold_value = TD_NULL;
    ot_unused(private_data);
    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    alpha_threshold_value = (td_u8 *)(uintptr_t)argp;

    if (tde_osi_get_alpha_threshold_value(alpha_threshold_value)) {
        return DRV_ERR_TDE_INVALID_PARA;
    }

    return TD_SUCCESS;
}

static td_s32 tde_osr_set_alpha_threshold_state(unsigned long argp, td_void *const private_data)
{
    td_bool alpha_threshold_en;
    ot_unused(private_data);
    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    alpha_threshold_en = *(td_bool *)(uintptr_t)argp;

    return tde_osi_set_alpha_threshold_state(alpha_threshold_en);
}

static td_s32 tde_osr_get_alpha_threshold_state(unsigned long argp, td_void *const private_data)
{
    td_bool alpha_threshold_en = TD_FALSE;
    ot_unused(private_data);
    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    (void)tde_osi_get_alpha_threshold_state(&alpha_threshold_en);

    *(td_bool *)(uintptr_t)argp = alpha_threshold_en;

    return TD_SUCCESS;
}

static td_s32 tde_osr_pattern_fill(unsigned long argp, td_void *const private_data)
{
    drv_tde_pattern_fill_opt *opt = TD_NULL;
    drv_tde_pattern_fill_cmd *pattern_fill_cmd = TD_NULL;
    drv_tde_double_src double_src = {0};
    ot_unused(private_data);

    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    pattern_fill_cmd = (drv_tde_pattern_fill_cmd *)(uintptr_t)argp;

    double_src.bg_surface = ((pattern_fill_cmd->null_indicator >> 1) & 1) ?
                             TD_NULL : &(pattern_fill_cmd->bg_surface);
    double_src.bg_rect = ((pattern_fill_cmd->null_indicator >> 2) & 1) ? /* 2 Take the high */
                          TD_NULL : &(pattern_fill_cmd->bg_rect);
    double_src.fg_surface = ((pattern_fill_cmd->null_indicator >> 3) & 1) ? /* 3 Take the high */
                            TD_NULL : &(pattern_fill_cmd->fg_surface);
    double_src.fg_rect = ((pattern_fill_cmd->null_indicator >> 4) & 1) ? /* 4 Take the high */
                          TD_NULL : &(pattern_fill_cmd->fg_rect);
    double_src.dst_surface = ((pattern_fill_cmd->null_indicator >> 5) & 1) ? /* 5 Take the high */
                              TD_NULL : &(pattern_fill_cmd->dst_surface);
    double_src.dst_rect = ((pattern_fill_cmd->null_indicator >> 6) & 1) ? /* 6 Take the high */
                           TD_NULL : &(pattern_fill_cmd->dst_rect);
    opt = ((pattern_fill_cmd->null_indicator >> 7) & 1) ? TD_NULL : /* 7 Take the high */
        &(pattern_fill_cmd->option);

    return tde_osi_pattern_fill(pattern_fill_cmd->handle, &double_src, opt);
}

static td_s32 tde_osr_enable_region_deflicker(unsigned long argp, td_void *const private_data)
{
    td_bool is_region_deflicker;
    ot_unused(private_data);

    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    is_region_deflicker = *(td_bool *)(uintptr_t)argp;

    return tde_osi_enable_region_deflicker(is_region_deflicker);
}

static td_s32 tde_osr_rotate(unsigned long argp, td_void *const private_data)
{
#if (TDE_CAPABILITY & ROTATE)
    drv_tde_rotate_cmd *rotate_cmd = TD_NULL;
    drv_tde_single_src single_src = {0};
    ot_unused(private_data);

    if (argp == 0) {
        return DRV_ERR_TDE_NULL_PTR;
    }
    rotate_cmd = (drv_tde_rotate_cmd *)(uintptr_t)argp;

    single_src.src_surface = &(rotate_cmd->src_surface);
    single_src.src_rect = &(rotate_cmd->src_rect);
    single_src.dst_surface = &(rotate_cmd->dst_surface);
    single_src.dst_rect = &(rotate_cmd->dst_rect);

    return tde_osi_quick_rotate(rotate_cmd->handle, &single_src, rotate_cmd->rotate_angle);
#else
    ot_unused(argp);
    ot_unused(private_data);
    return DRV_ERR_TDE_UNSUPPORTED_OPERATION;
#endif
}

static td_s32 tde_osr_bind_smmu(td_void)
{
    td_u32 sid = 0x1FU;
    td_s32 ssid;

    // config sid and ssid, ssv default enable
    ssid = svm_get_pasid(current->tgid, 0);
    if (ssid < 0) {
        tde_error("tde current pid is %d, get ssid failed ! please check if open svm in user\n", current->tgid);
        return TD_FAILURE;
    }
    tde_smmu_init(sid, (td_u32)ssid);
    return TD_SUCCESS;
}

static td_void tde_osr_unmap_mempool(td_void)
{
    td_s32 ret = TD_SUCCESS;
    struct media_mem_file* mem_file = tde_get_mem_file();

    ret = wmem_free_poolphy_addr();
    if (ret != TD_SUCCESS) {
        tde_error("tde free pool phy addr FAILED !!! \n");
    }

    media_mem_file_uninit(mem_file);
    osal_kfree(mem_file);
    mem_file = TD_NULL;
    tde_set_mem_file(TD_NULL);
    return;
}

static td_s32 tde_osr_map_mempool(td_void)
{
    td_s32 ret;
    struct media_mem_file *mem_file = (struct media_mem_file *)osal_kmalloc(sizeof(struct media_mem_file),
                                                                          osal_gfp_kernel);
    if (mem_file == TD_NULL) {
        return TD_FAILURE;
    }

    (td_void)memset_s(mem_file, sizeof(struct media_mem_file), 0, sizeof(struct media_mem_file));

    ret = media_mem_file_init(mem_file, current->tgid, 0, (hi_s32)HI_ID_TDE);
    if (ret != TD_SUCCESS) {
        osal_kfree(mem_file);
        mem_file = TD_NULL;
        return ret;
    }

    tde_set_mem_file(mem_file);

    ret = wmem_get_poolphy_addr();
    if (ret != TD_SUCCESS) {
        tde_error("tde get pool phy addr FAILED !!! \n");
        media_mem_file_uninit(mem_file);
        osal_kfree(mem_file);
        mem_file = TD_NULL;
        tde_set_mem_file(TD_NULL);
        return ret;
    }
    return TD_SUCCESS;
}

#ifdef TDE_COREDUMP_DEBUG
#define tde_read_reg(base, offset) (*(volatile unsigned int *)((td_void *)(base) + (offset)))
#endif

static int tde_osr_isr(int irq, td_void *const dev_id)
{
    td_u32 int_status;
    ot_unused(irq);
    ot_unused(dev_id);

#ifdef TDE_TIME_COUNT
    (td_void) osal_gettimeofday(&g_time_end);
    tde_info("tde_osr_isr: end time : %ld!\n", g_time_end.tv_sec * 1000000 + /* 1000000 us */
        g_time_end.tv_usec);
#endif
    int_status = tde_hal_ctl_int_status();
    if (int_status & TDE_DRV_INT_ERROR) {
#ifdef TDE_COREDUMP_DEBUG
        td_u32 read_stats;
        td_u32 i;
        for (i = 0; i < 74; i++) { /* 74 Error output */
            read_stats = tde_read_reg(tde_hal_get_base_vir_addr(), (0x800 + i * 4)); /* 4 alg data */
            tde_error("\n--------- ADDR:0x%x Value:0x%x---------\n", (0x800 + (i * 4)), read_stats); /* 4 alg data */
        }
#endif
        tde_error("tde interrupts coredump!\n");
        tde_hal_resume_init();

        return (td_s32)OSAL_IRQ_HANDLED;
    }

    if (int_status & TDE_DRV_INT_TIMEOUT) {
        tde_error("tde hardware timeout!\n");

        tde_hal_resume_init();
        return (td_s32)OSAL_IRQ_HANDLED;
    }

    tde_info("tde register int status: 0x%x!\n", int_status);

    tde_tasklet_func(int_status);

    return (td_s32)OSAL_IRQ_HANDLED;
}

static td_void tde_tasklet_func(unsigned long int_status)
{
    unsigned long lockflags;
    tde_spin_lock(&g_tasklet_lock, lockflags);

#ifndef __RTOS__
    tde_tasklet.data &= (~int_status);
#endif
    tde_spin_unlock(&g_tasklet_lock, lockflags);
    if (int_status & TDE_DRV_INT_NODE_COMP_AQ) {
        tde_osi_list_node_comp();
    }
}

td_s32 tde_osi_open(td_void)
{
    if (osal_atomic_inc_return(&g_hifb_use_count) > 0) {
        return tde_open(TD_NULL);
    }
    (void)osal_atomic_dec_return(&g_hifb_use_count);
    tde_error("hifb open tde err!\n");
    return TD_FAILURE;
}

td_s32 tde_osi_close(td_void)
{
    if (osal_atomic_dec_return(&g_hifb_use_count) >= 0) {
        return tde_release(TD_NULL);
    }
    (void)osal_atomic_inc_return(&g_hifb_use_count);
    tde_error("hifb close tde err! do not support multi close.\n");
    return TD_FAILURE;
}