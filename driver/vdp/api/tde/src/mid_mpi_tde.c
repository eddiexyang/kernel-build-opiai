/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: tde API mid file
 * Author: Hisilicon multimedia software group
 * Create: 2023/03/10
 */
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include "tde_define.h"
#include "mpi_tde_drv_para.h"
#include "mid_mpi_tde.h"

#define SVM_DEVICE_NAME "/dev/svm0"
#define SVM_IOCTL_PROCESS_BIND 0xffff

static int g_tde_fd = -1;
static int g_tde_ref = 0;
static pthread_mutex_t g_tde_mutex = PTHREAD_MUTEX_INITIALIZER;
struct svm_bind_process {
    pid_t vpid;
    unsigned long ttbr;
    unsigned long tcr;
    int pasid;
    unsigned int flags;
};

static inline void tde_lock(void)
{
    (void)pthread_mutex_lock(&g_tde_mutex);
}

static inline void tde_unlock(void)
{
    (void)pthread_mutex_unlock(&g_tde_mutex);
}

static int mid_mpi_svm_smmu_bind_tde(void)
{
    int fd;
    int ret;
    struct svm_bind_process bind_param;
    fd = open(SVM_DEVICE_NAME, O_RDWR);
    if (fd < 0) {
        tde_error("open svm fail, svm_name:%s\n", SVM_DEVICE_NAME);
        return HI_ERR_TDE_DEV_OPEN_FAILED;
    }
    bind_param.flags = 0;
    ret = ioctl(fd, SVM_IOCTL_PROCESS_BIND, &bind_param);
    if (ret < 0) {
        tde_error("svm ioctl bind fail, error no [%d]\n", ret);
        close(fd);
        return HI_ERR_TDE_DEV_OPEN_FAILED;
    }
    close(fd);
    return MID_SUCCESS;
}

int mid_tde_open(void)
{
    int ret;
    tde_lock();
    if (g_tde_fd != -1) {
        g_tde_ref++;
        tde_unlock();
        return MID_SUCCESS;
    }

#ifdef ENV_SMMU_SUPPORT
    ret = mid_mpi_svm_smmu_bind_tde();
    if (ret != MID_SUCCESS) {
        tde_unlock();
        return ret;
    }
#endif

    g_tde_fd = open("/dev/ot_tde", O_RDWR, S_IRUSR);
    if (g_tde_fd < 0) {
        tde_unlock();
        return HI_ERR_TDE_DEV_OPEN_FAILED;
    }

    g_tde_ref++;
    tde_unlock();

    return MID_SUCCESS;
}

void mid_tde_close(void)
{
    tde_lock();
    if (g_tde_fd == -1) {
        tde_unlock();
        return;
    }
    g_tde_ref--;

    if (g_tde_ref > 0) {
        tde_unlock();
        return;
    } else {
        g_tde_ref = 0;
    }

    close(g_tde_fd);
    g_tde_fd = -1;
    tde_unlock();
    return;
}

int mid_tde_begin_job(void)
{
    int ret;
    int handle = HI_ERR_TDE_INVALID_HANDLE;

    if (g_tde_fd == -1) {
        return HI_ERR_TDE_INVALID_HANDLE;
    }

    ret = ioctl(g_tde_fd, TDE_BEGIN_JOB, &handle);
    if (ret < 0) {
        return HI_ERR_TDE_INVALID_HANDLE;
    }

    return handle;
}

int mid_tde_end_job(int handle, mid_bool is_sync, mid_bool is_block, unsigned int time_out)
{
    drv_tde_end_job_cmd end_job_cmd = {0};
    if (g_tde_fd == -1) {
        return HI_ERR_TDE_DEV_NOT_OPEN;
    }
    end_job_cmd.handle = handle;
    end_job_cmd.is_sync = (td_bool)is_sync;
    end_job_cmd.is_block = (td_bool)is_block;
    end_job_cmd.time_out = time_out;

    return ioctl(g_tde_fd, TDE_END_JOB, &end_job_cmd);
}

int mid_tde_cancel_job(int handle)
{
    if (g_tde_fd == -1) {
        return HI_ERR_TDE_DEV_NOT_OPEN;
    }

    return ioctl(g_tde_fd, TDE_CANCEL_JOB, &handle);
}

int mid_tde_wait_for_done(int handle)
{
    if (g_tde_fd == -1) {
        return HI_ERR_TDE_DEV_NOT_OPEN;
    }

    return ioctl(g_tde_fd, TDE_WAITFORDONE, &handle);
}

int mid_tde_wait_all_done(void)
{
    if (g_tde_fd == -1) {
        return HI_ERR_TDE_DEV_NOT_OPEN;
    }

    return ioctl(g_tde_fd, TDE_WAITALLDONE);
}


int mid_tde_quick_fill(int handle, const mid_tde_none_src *none_src, unsigned int fill_data)
{
    drv_tde_quick_fill_cmd quick_fill_cmd = {0};
    if (g_tde_fd == -1) {
        return HI_ERR_TDE_DEV_NOT_OPEN;
    }

    if ((none_src == NULL) || (none_src->dst_surface == NULL) || (none_src->dst_rect == NULL)) {
        return HI_ERR_TDE_NULL_PTR;
    }

    quick_fill_cmd.handle = handle;
    quick_fill_cmd.fill_data = fill_data;

    mpi_convert_surface_user_to_drv(none_src->dst_surface, &(quick_fill_cmd.dst_surface));
    mpi_convert_rect_user_to_drv(none_src->dst_rect, &(quick_fill_cmd.dst_rect));

    return ioctl(g_tde_fd, TDE_QUICK_FILL, &quick_fill_cmd);
}

int mid_tde_quick_copy(int handle, const mid_tde_single_src *single_src)
{
    drv_tde_quick_copy_cmd quick_copy_cmd = {0};
    if (g_tde_fd == -1) {
        return HI_ERR_TDE_DEV_NOT_OPEN;
    }

    if ((single_src == NULL) || (single_src->src_surface == NULL) || (single_src->src_rect == NULL) ||
        (single_src->dst_surface == NULL) || (single_src->dst_rect == NULL)) {
        return HI_ERR_TDE_NULL_PTR;
    }

    quick_copy_cmd.handle = handle;

    mpi_convert_surface_user_to_drv(single_src->src_surface, &(quick_copy_cmd.src_surface));
    mpi_convert_rect_user_to_drv(single_src->src_rect, &(quick_copy_cmd.src_rect));

    mpi_convert_surface_user_to_drv(single_src->dst_surface, &(quick_copy_cmd.dst_surface));
    mpi_convert_rect_user_to_drv(single_src->dst_rect, &(quick_copy_cmd.dst_rect));

    return ioctl(g_tde_fd, TDE_QUICK_COPY, &quick_copy_cmd);
}

static int mid_tde_pattern_fill_param_check(mid_tde_pattern_fill_opt *fill_opt)
{
    if (fill_opt->alpha_blending_cmd != HI_TDE_ALPHA_BLENDING_BLEND) {
        tde_error("tde pattern_fill param check failed cmd is %d, cur only support alpha blending",
                  fill_opt->alpha_blending_cmd);
        return HI_ERR_TDE_INVALID_PARAM;
    }
    fill_opt->colorkey_mode = HI_TDE_COLORKEY_MODE_NONE;
    fill_opt->clip_mode = HI_TDE_CLIP_MODE_NONE;
    fill_opt->clut_reload = MID_FALSE;
    (void)memset_s(&fill_opt->csc_opt, sizeof(mid_tde_csc_opt), 0x0, sizeof(mid_tde_csc_opt));
    return TD_SUCCESS;
}

int mid_tde_pattern_fill(int handle, const mid_tde_double_src *double_src, const mid_tde_pattern_fill_opt *fill_opt)
{
    drv_tde_pattern_fill_cmd pattern_fill_cmd = {0};
    if (g_tde_fd == -1) {
        return HI_ERR_TDE_DEV_NOT_OPEN;
    }

    if (double_src == NULL) {
        return HI_ERR_TDE_NULL_PTR;
    }

    pattern_fill_cmd.handle = handle;

    if (double_src->bg_surface == NULL) {
        pattern_fill_cmd.null_indicator |= (1U << 1U);
    } else {
        mpi_convert_surface_user_to_drv(double_src->bg_surface, &(pattern_fill_cmd.bg_surface));
    }

    if (double_src->bg_rect == NULL) {
        pattern_fill_cmd.null_indicator |= (1U << 2U); /* 2 for calculate */
    } else {
        mpi_convert_rect_user_to_drv(double_src->bg_rect, &(pattern_fill_cmd.bg_rect));
    }

    if (double_src->fg_surface == NULL) {
        pattern_fill_cmd.null_indicator |= (1U << 3U); /* 3 for calculate */
    } else {
        mpi_convert_surface_user_to_drv(double_src->fg_surface, &(pattern_fill_cmd.fg_surface));
    }

    if (double_src->fg_rect == NULL) {
        pattern_fill_cmd.null_indicator |= (1U << 4U); /* 4 for calculate */
    } else {
        mpi_convert_rect_user_to_drv(double_src->fg_rect, &(pattern_fill_cmd.fg_rect));
    }

    if (double_src->dst_surface == NULL) {
        pattern_fill_cmd.null_indicator |= (1U << 5U); /* 5 for calculate */
    } else {
        mpi_convert_surface_user_to_drv(double_src->dst_surface, &(pattern_fill_cmd.dst_surface));
    }

    if (double_src->dst_rect == NULL) {
        pattern_fill_cmd.null_indicator |= (1U << 6U); /* 6 for calculate */
    } else {
        mpi_convert_rect_user_to_drv(double_src->dst_rect, &(pattern_fill_cmd.dst_rect));
    }

    if (fill_opt == NULL) {
        pattern_fill_cmd.null_indicator |= (1U << 7U); /* 7 for calculate */
    } else {
        mid_tde_pattern_fill_opt cur_opt = *fill_opt;
        int ret = mid_tde_pattern_fill_param_check(&cur_opt);
        if (ret != TD_SUCCESS) {
            return ret;
        }
        mpi_convert_pattern_fill_option_user_to_drv(&cur_opt, &(pattern_fill_cmd.option));
    }

    return ioctl(g_tde_fd, TDE_PATTERN_FILL, &pattern_fill_cmd);
}
