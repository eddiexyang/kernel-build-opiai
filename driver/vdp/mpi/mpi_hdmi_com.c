/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2022. All rights reserved.
 * Description: Common definition of HDMI
 * Author: Hisilicon multimedia software group
 * Create: 2019/07/06
 */
#include "mpi_hdmi_com.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ot_common.h>
#include "drv_hdmi_ioctl.h"
#include "list.h"
#include "ot_math.h"
#include "mkp_hdmi.h"

#define UMAP_DEVNAME_HDMI    "hdmi"
#define MAX_DELAY_TIME_MS    10000
#define TV_SEC_MULTIPLE      1000
#define THREAD_FUNC_NAMR_LEN 32
#define THREAD_EXIT_TIMEOUT  200
#define THEAD_FUNC_NAME      "HDMI_Event"

#define hdmi_check_lock_return(mutex, ret)                      \
    do {                                                        \
        if (pthread_mutex_lock(&(mutex)) != 0) {                \
            hdmi_debug_trace("pthread mutex lock is failed!\n"); \
            return ret;                                         \
        }                                                       \
    } while (0)

#define hdmi_unlock(mutex)                    \
    do {                                      \
        (void)pthread_mutex_unlock(&(mutex)); \
    } while (0)

#define hdmi_check_init_return(dev, mutex)              \
    do {                                                \
        if ((dev) < 0) {                                \
            hdmi_fatal_trace("HDMI device not init\n"); \
            hdmi_unlock(mutex);                         \
            return OT_ERR_HDMI_NOT_INIT;                \
        }                                               \
    } while (0)

#define hdmi_check_chn_open_return(hdmi_id, mutex)                       \
    do {                                                                 \
        if (g_hdmi_chn_user_param[(hdmi_id)].open != TD_TRUE) {          \
            hdmi_warn_trace("hdmi:%u do NOT open\n", (td_u32)(hdmi_id)); \
            hdmi_unlock(mutex);                                          \
            return OT_ERR_HDMI_DEV_NOT_OPEN;                             \
        }                                                                \
    } while (0)

#define hdmi_check_id_return(hdmi_id)                                      \
    do {                                                                   \
        if ((hdmi_id) >= HDMI_MPI_ID_MAX) {                                \
            hdmi_err_trace("hdmi_id %u is invalid.\n", (td_u32)(hdmi_id)); \
            return OT_ERR_HDMI_INVALID_PARA;                               \
        }                                                                  \
    } while (0)

#define hdmi_check_failure_return(ret)                     \
    do {                                                   \
        if ((ret) == TD_FAILURE) {                         \
            hdmi_err_trace("Hdmi Kernel Strategy fail\n"); \
            ret = OT_ERR_HDMI_STRATEGY_FAILED;             \
        }                                                  \
    } while (0)

typedef struct {
    struct list_head list;
    ot_hdmi_callback_func callback_func;
} hdmi_callback_node;

typedef struct {
    td_bool open;
    td_bool start;
    ot_hdmi_id hdmi_id;
    hdmi_base_attr attr;
    ot_hdmi_avi_infoframe avi_infoframe;
    ot_hdmi_audio_infoframe audio_infoframe;
} hdmi_chn_user_attr;

typedef struct {
    td_bool hdmi_init;
    td_bool enable_timer;  /* Timer thread Flag */
    pthread_t event_timer; /* Timer thread ID */
    td_bool hdmi_exit;     /* HDMI Exit Flag */
    td_u32 param;          /* Timer thread Param */
} hdmi_comm_user_attr;

static td_s32              g_hdmi_dev_fd = -1;
static hdmi_callback_node  g_hdmi_callback_list[(td_s32)HDMI_MPI_ID_MAX];
static hdmi_comm_user_attr g_hdmi_comm_user_param[(td_s32)HDMI_MPI_ID_MAX];
static hdmi_chn_user_attr  g_hdmi_chn_user_param[(td_s32)HDMI_MPI_ID_MAX];
static pthread_mutex_t     g_hdmi_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t     g_callback_mutex = PTHREAD_MUTEX_INITIALIZER;
static td_bool             g_event_thread_run = TD_FALSE;

static td_u64 mpi_hdmi_get_time_ms(td_void)
{
    td_u64 time;
    struct timeval tv = {0};

    gettimeofday(&tv, TD_NULL);
    time = ((td_u64)tv.tv_sec * (td_u64)TV_SEC_MULTIPLE) + ((td_u64)tv.tv_usec / (td_u64)TV_SEC_MULTIPLE);

    return time;
}

static void hdmi_event_callback_invoke(ot_hdmi_id hdmi, ot_hdmi_event_type event)
{
    ot_hdmi_id private_data;
    hdmi_callback_node *tmp = TD_NULL;
    struct list_head *pos = TD_NULL;
    struct list_head *q = TD_NULL;

    if (hdmi >= HDMI_MPI_ID_MAX) {
        hdmi_err_trace("hdmi %d, event %d is invalid\n", (td_s32)hdmi, (td_s32)event);
        return;
    }
    /* camera only support HPD/UNHPD */
    if (event != OT_HDMI_EVENT_HOTPLUG && event != OT_HDMI_EVENT_NO_PLUG) {
        return;
    }
    if (pthread_mutex_lock(&g_callback_mutex) != 0) {
        hdmi_debug_trace("pthread mutex lock is failed!\n");
        return;
    }
    private_data = hdmi;
    list_for_each_safe(pos, q, &g_hdmi_callback_list[hdmi].list) {
        tmp = list_entry(pos, hdmi_callback_node, list);
        if (tmp != TD_NULL && tmp->callback_func.hdmi_event_callback != TD_NULL) {
            tmp->callback_func.private_data = &private_data;
            tmp->callback_func.hdmi_event_callback(event, tmp->callback_func.private_data);
        }
    }
    hdmi_unlock(g_callback_mutex);

    return;
}

static ot_hdmi_event_type event_type_drv2user(hdmi_event kernel_event)
{
    ot_hdmi_event_type ret;

    switch (kernel_event) {
        case HDMI_EVENT_HOTPLUG:
            ret = OT_HDMI_EVENT_HOTPLUG;
            break;
        case HDMI_EVENT_HOTUNPLUG:
            ret = OT_HDMI_EVENT_NO_PLUG;
            break;
        case HDMI_EVENT_EDID_FAIL:
            ret = OT_HDMI_EVENT_EDID_FAIL;
            break;
        default:
            ret = HDMI_EVENT_BUTT;
            break;
    }

    return ret;
}

static td_void *hdmi_event_poll_thread(void *param)
{
    td_s32 ret;
    ot_hdmi_id hdmi;
    ot_hdmi_event_type event_type;
    drv_hdmi_event curr_event = {0};
    td_char func_name[THREAD_FUNC_NAMR_LEN] = {0};

    ret = snprintf_s(func_name, THREAD_FUNC_NAMR_LEN, 14, "%s", THEAD_FUNC_NAME); /* 14, the max len of thread name */
    if (ret < 0) {
        hdmi_err_trace("snprintf_s err\n");
        return TD_NULL;
    }

    prctl(PR_SET_NAME, (uintptr_t)func_name, 0, 0, 0);
    if (param == TD_NULL) {
        hdmi_err_trace("param is invalid!\n");
        g_event_thread_run = TD_FALSE;
        return TD_NULL;
    }
    hdmi = *(ot_hdmi_id *)param;
    if (hdmi >= HDMI_MPI_ID_MAX) {
        hdmi_err_trace("hdmi %d is invalid!\n", (td_s32)hdmi);
        g_event_thread_run = TD_FALSE;
        return TD_NULL;
    }
    g_event_thread_run = TD_TRUE;
    while (g_hdmi_comm_user_param[hdmi].hdmi_exit == TD_FALSE) {
        if (g_hdmi_chn_user_param[hdmi].open == TD_FALSE) {
            ot_usleep(100 * 1000); /* 100, 1000, sleep 100ms if hdmi not open */
            continue;
        }
        (td_void)memset_s(&curr_event, sizeof(curr_event), 0, sizeof(curr_event));
        curr_event.hdmi_id = hdmi;
        ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_POLL_EVENT, &curr_event);
        if (ret == TD_SUCCESS && curr_event.event != HDMI_EVENT_BUTT) {
            event_type = event_type_drv2user(curr_event.event);
            hdmi_event_callback_invoke(hdmi, event_type);
        }
        ot_usleep(10 * 1000); /* 10, 1000, event read interval 10ms */
    }
    g_event_thread_run = TD_FALSE;

    return TD_NULL;
}

static td_s32 creat_event_poll_pthread(ot_hdmi_id hdmi)
{
    td_s32         ret;
    td_void       *param = TD_NULL;
    pthread_attr_t thread_attr = {0};

    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    g_hdmi_comm_user_param[hdmi].param = (td_u32)hdmi;
    param = (td_void *)(&(g_hdmi_comm_user_param[hdmi].param));
    ret = pthread_create(&g_hdmi_comm_user_param[hdmi].event_timer, &thread_attr, hdmi_event_poll_thread, param);
    pthread_attr_destroy(&thread_attr);

    return ret;
}

td_s32 mpi_hdmi_com_init(void)
{
    td_u32 hdmi_id;
    ot_hdmi_id hdmi = OT_HDMI_ID_0;
    const td_char hdmi_dev_name[] = "/dev/"UMAP_DEVNAME_HDMI;

    if (pthread_mutex_lock(&g_hdmi_mutex) != 0) {
        if (pthread_mutex_init(&g_hdmi_mutex, NULL) != TD_SUCCESS) {
            hdmi_debug_trace("g_hdmi_mutex init error!\n");
            goto exit;
        }
        hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    }
    if (g_hdmi_dev_fd >= 0 || g_hdmi_chn_user_param[hdmi].open == TD_TRUE) {
        hdmi_unlock(g_hdmi_mutex);
        return TD_SUCCESS;
    }
    if (g_event_thread_run == TD_FALSE) {
        for (hdmi_id = 0; hdmi_id < HDMI_MPI_ID_MAX; hdmi_id++) {
            INIT_LIST_HEAD(&g_hdmi_callback_list[hdmi_id].list);
        }
    }
    (td_void)memset_s(&g_hdmi_chn_user_param, sizeof(g_hdmi_chn_user_param), 0, sizeof(g_hdmi_chn_user_param));
    (td_void)memset_s(&g_hdmi_comm_user_param, sizeof(g_hdmi_comm_user_param), 0, sizeof(g_hdmi_comm_user_param));
    g_hdmi_dev_fd = open(hdmi_dev_name, O_RDWR);
    if (g_hdmi_dev_fd < 0) {
        hdmi_fatal_trace("open HDMI err.\n");
        hdmi_unlock(g_hdmi_mutex);
        goto exit;
    }
    hdmi_unlock(g_hdmi_mutex);

    return TD_SUCCESS;

exit:
    return OT_ERR_HDMI_INIT_FAILED;
}

td_s32 mpi_hdmi_com_deinit(void)
{
    td_s32 ret = 0;
    td_u64 start_time;
    td_s32 hdmi_id;

    if (pthread_mutex_lock(&g_hdmi_mutex) != 0) {
        hdmi_debug_trace("pthread mutex lock is failed!!\n");
        return OT_ERR_HDMI_MUTEX_LOCK_FAILED;
    }
    if (g_hdmi_dev_fd < 0) {
        hdmi_unlock(g_hdmi_mutex);
        return TD_SUCCESS;
    }

    for (hdmi_id = 0; hdmi_id < (td_s32)HDMI_MPI_ID_MAX; hdmi_id++) {
        if (g_hdmi_chn_user_param[hdmi_id].open != TD_TRUE) {
            continue;
        }

        if (g_hdmi_chn_user_param[hdmi_id].start == TD_TRUE) {
            ret += (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_STOP, &hdmi_id);
            g_hdmi_chn_user_param[hdmi_id].start = TD_FALSE;
        }
        g_hdmi_chn_user_param[hdmi_id].open = TD_FALSE;
        g_hdmi_comm_user_param[hdmi_id].hdmi_exit = TD_TRUE;

        if (g_hdmi_comm_user_param[hdmi_id].enable_timer == TD_TRUE) {
            g_hdmi_comm_user_param[hdmi_id].enable_timer = TD_FALSE;
            start_time = mpi_hdmi_get_time_ms();
            /* wait thread exit and the resources recycled by system. */
            while (g_event_thread_run == TD_TRUE && ((mpi_hdmi_get_time_ms() - start_time) <= THREAD_EXIT_TIMEOUT)) {};
        }

        ret += (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_CLOSE, &hdmi_id);
        ret = (ret != TD_SUCCESS) ? TD_FAILURE : TD_SUCCESS;
        if (ret != TD_SUCCESS) {
            hdmi_unlock(g_hdmi_mutex);
            hdmi_check_failure_return(ret);
            return ret;
        }
    }

    close(g_hdmi_dev_fd);
    g_hdmi_dev_fd = -1;
    (td_void)memset_s(&g_hdmi_comm_user_param, sizeof(g_hdmi_comm_user_param), 0, sizeof(g_hdmi_comm_user_param));
    hdmi_unlock(g_hdmi_mutex);

    return TD_SUCCESS;
}

td_s32 mpi_hdmi_com_open(ot_hdmi_id hdmi, const hdmi_open *open_param)
{
    td_s32    ret;
    td_u64    start_time;
    hdmi_open open = {0};

    hdmi_check_id_return(hdmi);
    if (pthread_mutex_lock(&g_hdmi_mutex) != 0) {
        hdmi_debug_trace("pthread mutex lock is failed!\n");
        return OT_ERR_HDMI_NOT_INIT;
    }
    hdmi_check_init_return(g_hdmi_dev_fd, g_hdmi_mutex);
    if (g_hdmi_chn_user_param[hdmi].open) {
        hdmi_unlock(g_hdmi_mutex);
        return TD_SUCCESS;
    }

    ret = creat_event_poll_pthread(hdmi);
    if (ret != TD_SUCCESS) {
        hdmi_fatal_trace("timer task return:0x%x\n", ret);
        hdmi_unlock(g_hdmi_mutex);
        return OT_ERR_HDMI_PTHREAD_CREATE_FAILED;
    }
    g_hdmi_comm_user_param[hdmi].enable_timer = TD_TRUE;
    open.hdmi_id = hdmi;
    open.default_mode = HDMI_DEFAULT_ACTION_HDMI;
    if (open_param != TD_NULL) {
        open.default_mode = open_param->default_mode;
    }
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_OPEN, &open);
    if (ret != TD_SUCCESS) {
        hdmi_debug_trace("hdmi open err:0x%x\n", ret);
        g_hdmi_comm_user_param[hdmi].enable_timer = TD_FALSE;
        g_hdmi_comm_user_param[hdmi].hdmi_exit = TD_TRUE;
        start_time = mpi_hdmi_get_time_ms();
        while (g_event_thread_run == TD_TRUE) {
            if ((mpi_hdmi_get_time_ms() - start_time) > 120) { /* 120, wait thread exit time(ms) */
                hdmi_err_trace("Thread exit timeout: %d\n", (td_s32)g_event_thread_run);
                break;
            }
        }
        hdmi_unlock(g_hdmi_mutex);
        hdmi_check_failure_return(ret);

        return ret;
    }
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_UNREGISTER_CALLBACK, &hdmi);
    if (ret != TD_SUCCESS) {
        hdmi_unlock(g_hdmi_mutex);
        hdmi_check_failure_return(ret);
        return ret;
    }

    g_hdmi_chn_user_param[hdmi].open = TD_TRUE;
    hdmi_unlock(g_hdmi_mutex);

    return TD_SUCCESS;
}

td_s32 mpi_hdmi_com_close(ot_hdmi_id hdmi)
{
    td_s32 ret = TD_SUCCESS;
    td_u64 start_time;

    hdmi_check_id_return(hdmi);

    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);

    if (g_hdmi_chn_user_param[hdmi].open != TD_TRUE) {
        hdmi_unlock(g_hdmi_mutex);
        return ret;
    }
    if (g_hdmi_chn_user_param[hdmi].start == TD_TRUE) {
        ret += (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_STOP, &hdmi);
        g_hdmi_chn_user_param[hdmi].start = TD_FALSE;
    }
    g_hdmi_chn_user_param[hdmi].open = TD_FALSE;
    g_hdmi_comm_user_param[hdmi].hdmi_exit = TD_TRUE;
    if (g_hdmi_comm_user_param[hdmi].enable_timer == TD_TRUE) {
        g_hdmi_comm_user_param[hdmi].enable_timer = TD_FALSE;
        start_time = mpi_hdmi_get_time_ms();
        /* wait thread exit and the resources recycled by system. */
        while (g_event_thread_run == TD_TRUE) {
            if ((mpi_hdmi_get_time_ms() - start_time) > 200) { /* 200, wait thread exit time(ms) */
                break;
            }
        }
    }
    ret += (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_CLOSE, &hdmi);
    ret = (ret != TD_SUCCESS) ? TD_FAILURE : TD_SUCCESS;
    if (ret != TD_SUCCESS) {
        hdmi_unlock(g_hdmi_mutex);
        hdmi_check_failure_return(ret);
        return ret;
    }
    hdmi_unlock(g_hdmi_mutex);

    return ret;
}
#ifdef HDMI_DFX_SUPPORT
td_s32 mpi_hdmi_set_timing(ot_hdmi_id hdmi, drv_hdmi_timing_format timing_format)
{
    hdmi_check_id_return(hdmi);
    td_s32 ret = TD_SUCCESS;
    drv_hdmi_timing_format *hdmi_timing_format = &timing_format;

    ret += (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_SET_TIMING, hdmi_timing_format);
    return ret;
}

td_s32 mpi_hdmi_reset_clock(ot_hdmi_id hdmi, drv_hdmi_reset_clk reset_clk)
{
    hdmi_check_id_return(hdmi);
    td_s32 ret = TD_SUCCESS;
    ret += (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_RESET_CLOCK, &reset_clk);
    return ret;
}

td_s32 mpi_hdmi_set_pattern(ot_hdmi_id hdmi, drv_hdmi_pattern_mode pattern_mode)
{
    hdmi_check_id_return(hdmi);
    td_s32 ret = TD_SUCCESS;
    ret += (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_SET_PATTERN, &pattern_mode);
    return ret;
}

td_s32 mpi_hdmi_set_hotplug(ot_hdmi_id hdmi, drv_hdmi_hotplug_status *hotplug_status)
{
    hdmi_check_id_return(hdmi);
    if (hotplug_status == TD_NULL) {
        hdmi_err_trace("the pointer of hotplug_status is null.\n");
        return TD_FAILURE;
    }
    td_s32 ret = TD_SUCCESS;
    drv_hdmi_hotplug_status hdmi_hotplug_status;

    ret += (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_SET_HOTPLUG, &hdmi_hotplug_status);
    hotplug_status->hotplug_on = hdmi_hotplug_status.hotplug_on;
    hdmi_debug_trace("mpi_hdmi_hotplug== %u\n", hotplug_status->hotplug_on);

    return ret;
}

td_s32 mpi_hdmi_set_detection(ot_hdmi_id hdmi, drv_hdmi_detection *hdmi_detection)
{
    hdmi_check_id_return(hdmi);
    if (hdmi_detection == TD_NULL) {
        hdmi_err_trace("the pointer of hdmi_detection is null.\n");
        return TD_FAILURE;
    }
    td_s32 ret = TD_SUCCESS;
    drv_hdmi_detection detection;

    ret += (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_SET_DETECTION, &detection);
    hdmi_detection->timing_height = detection.timing_height;
    hdmi_detection->timing_width = detection.timing_width;
    hdmi_debug_trace("mpi_hdmi_set_detection= %u , %u\n", hdmi_detection->timing_height,
                     hdmi_detection->timing_width);
    return ret;
}

td_s32 mpi_hdmi_set_capture(ot_hdmi_id hdmi, drv_hdmi_capture *hdmi_capture)
{
    hdmi_check_id_return(hdmi);
    if (hdmi_capture == TD_NULL) {
        hdmi_err_trace("the pointer of hdmi_capture is null.\n");
        return TD_FAILURE;
    }
    td_s32 ret = TD_SUCCESS;
    drv_hdmi_capture capture;
    capture.pos_x = hdmi_capture->pos_x;
    capture.pos_y = hdmi_capture->pos_y;
    hdmi_debug_trace("capture.pos_x== %u,capture.pos_y= %u\n", capture.pos_x, capture.pos_x);
    ret += (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_SET_CAPTURE, &capture);
    hdmi_capture->color_b = capture.color_b;
    hdmi_capture->color_g = capture.color_g;
    hdmi_capture->color_r = capture.color_r;

    hdmi_debug_trace("hdmi_capture->color_b== %u,hdmi_capture->color_g== %u,hdmi_capture->color_r== %u\n",
                     hdmi_capture->color_b, hdmi_capture->color_b, hdmi_capture->color_r);
    return ret;
}

td_s32 mpi_hdmi_avi_infoframe(drv_hdmi_infoframe_colorspace infoframe_colorspace)
{
    ot_hdmi_id hdmi = infoframe_colorspace.id;
    hdmi_check_id_return(hdmi);
    td_s32 ret = TD_SUCCESS;

    drv_hdmi_infoframe_colorspace *colorspace = &infoframe_colorspace;
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_AVI_INFOFRAME, colorspace);
    return ret;
}
#endif

td_s32 mpi_hdmi_com_start(ot_hdmi_id hdmi)
{
    td_s32 ret;

    hdmi_check_id_return(hdmi);
    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);

    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_START, &hdmi);
    if (ret != TD_SUCCESS) {
        hdmi_unlock(g_hdmi_mutex);
        hdmi_check_failure_return(ret);
        return ret;
    }
    g_hdmi_chn_user_param[hdmi].start = TD_TRUE;
    hdmi_unlock(g_hdmi_mutex);

    return TD_SUCCESS;
}

td_s32 mpi_hdmi_com_stop(ot_hdmi_id hdmi)
{
    td_s32 ret;

    hdmi_check_id_return(hdmi);
    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);

    if (g_hdmi_chn_user_param[hdmi].start != TD_TRUE) {
        hdmi_unlock(g_hdmi_mutex);
        return TD_SUCCESS;
    }
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_STOP, &hdmi);
    if (ret != TD_SUCCESS) {
        hdmi_unlock(g_hdmi_mutex);
        hdmi_check_failure_return(ret);
        return ret;
    }
    g_hdmi_chn_user_param[hdmi].start = TD_FALSE;
    hdmi_unlock(g_hdmi_mutex);

    return TD_SUCCESS;
}

td_s32 mpi_hdmi_com_set_infoframe(ot_hdmi_id hdmi, drv_hdmi_infoframe *user_infoframe)
{
    td_s32 ret;

    hdmi_check_id_return(hdmi);
    hdmi_check_null_return(user_infoframe, OT_ERR_HDMI_NULL_PTR);
    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);

    user_infoframe->hdmi_id = hdmi;
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_SET_INFOFRAME, user_infoframe);
    hdmi_unlock(g_hdmi_mutex);
    if (ret != TD_SUCCESS) {
        hdmi_check_failure_return(ret);
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 mpi_hdmi_com_get_infoframe(ot_hdmi_id hdmi, drv_hdmi_infoframe *drv_infoframe)
{
#ifdef HDMI_DFX_SUPPORT
    td_s32 ret;

    hdmi_check_id_return(hdmi);
    hdmi_check_null_return(drv_infoframe, OT_ERR_HDMI_NULL_PTR);
    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);

    drv_infoframe->hdmi_id = hdmi;
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_GET_INFOFRAME, drv_infoframe);
    hdmi_unlock(g_hdmi_mutex);
    if (ret != TD_SUCCESS) {
        hdmi_check_failure_return(ret);
        return ret;
    }

    return ret;
#else
    ot_unused(hdmi);
    ot_unused(drv_infoframe);
    return TD_SUCCESS;
#endif
}

td_s32 mpi_hdmi_com_get_status(ot_hdmi_id hdmi, ot_hdmi_status *status)
{
    td_s32 ret;
    drv_hdmi_status drv_status = {0};

    hdmi_check_id_return(hdmi);
    hdmi_check_null_return(status, OT_ERR_HDMI_NULL_PTR);
    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);

    (td_void)memset_s(&drv_status, sizeof(drv_hdmi_status), 0, sizeof(drv_hdmi_status));
    drv_status.hdmi_id = hdmi;
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_GET_STATUS, &drv_status);
    if (ret != TD_SUCCESS) {
        hdmi_unlock(g_hdmi_mutex);
        hdmi_check_failure_return(ret);
        return ret;
    }
    status->is_connected = drv_status.status.connected;
    status->is_sink_power_on = drv_status.status.sink_power_on;
    hdmi_unlock(g_hdmi_mutex);

    return ret;
}

td_s32 mpi_hdmi_com_get_sink_capability(ot_hdmi_id hdmi, drv_hdmi_sink_capability *drv_cap)
{
    td_s32 ret;

    hdmi_check_id_return(hdmi);
    hdmi_check_null_return(drv_cap, OT_ERR_HDMI_NULL_PTR);
    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);

    drv_cap->hdmi_id = hdmi;
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_GET_SINK_CAPABILITY, drv_cap);
    hdmi_unlock(g_hdmi_mutex);
    hdmi_check_failure_return(ret);

    return ret;
}

td_s32 mpi_hdmi_com_force_get_edid(ot_hdmi_id hdmi, td_u8 *edid, td_u32 *edid_len)
{
#ifdef HDMI_DFX_SUPPORT
    td_s32 ret;
    drv_hdmi_edid_raw_data edid_data = {0};

    hdmi_check_id_return(hdmi);
    hdmi_check_null_return(edid, OT_ERR_HDMI_NULL_PTR);
    hdmi_check_null_return(edid_len, OT_ERR_HDMI_NULL_PTR);
    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);

    *edid_len = 0;
    edid_data.hdmi_id = hdmi;
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_UPDATE_EDID, &edid_data);
    hdmi_unlock(g_hdmi_mutex);
    if ((ret == (td_s32)TD_SUCCESS) && (edid_data.edid_raw.edid_valid == (td_s32)TD_TRUE) &&
        (edid_data.edid_raw.edid_len != 0)) {
        *edid_len = (edid_data.edid_raw.edid_len > HDMI_EDID_SIZE) ? HDMI_EDID_SIZE : edid_data.edid_raw.edid_len;
        /* the maximum of edid size (4 blocks, each block has 128 bytes) */
        ret = memcpy_s(edid, HDMI_EDID_SIZE, edid_data.edid_raw.edid, *edid_len);
        if (ret != EOK) {
            hdmi_err_trace("memcpy_s fail!\n");
            return OT_ERR_HDMI_INVALID_PARA;
        }
    } else {
        hdmi_err_trace("Force get edid fail!\n");
        ret = OT_ERR_HDMI_READ_EDID_FAILED;
    }

    return ret;
#else
    ot_unused(hdmi);
    ot_unused(edid);
    ot_unused(edid_len);
    return TD_SUCCESS;
#endif
}

td_s32 mpi_hdmi_com_reg_callback_func(ot_hdmi_id hdmi, const ot_hdmi_callback_func *callback_func)
{
    td_s32 ret;
    hdmi_callback_node *tmp = NULL;
    struct list_head *pos = TD_NULL;
    struct list_head *q = TD_NULL;

    hdmi_check_id_return(hdmi);
    hdmi_check_null_return(callback_func, OT_ERR_HDMI_NULL_PTR);
    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);
    hdmi_unlock(g_hdmi_mutex);

    hdmi_check_lock_return(g_callback_mutex, OT_ERR_HDMI_MUTEX_LOCK_FAILED);
    if (callback_func->hdmi_event_callback != TD_NULL) {
        list_for_each_safe(pos, q, &g_hdmi_callback_list[hdmi].list) {
            tmp = list_entry(pos, hdmi_callback_node, list);
            if (tmp != TD_NULL &&
                tmp->callback_func.hdmi_event_callback == callback_func->hdmi_event_callback) {
                hdmi_warn_trace("This callback has been registered.\n");
                hdmi_unlock(g_callback_mutex);
                return OT_ERR_HDMI_CALLBACK_ALREADY;
            }
        }
        tmp = (hdmi_callback_node *)malloc(sizeof(hdmi_callback_node));
        if (tmp == TD_NULL) {
            hdmi_err_trace("Callback malloc failed\n");
            hdmi_unlock(g_callback_mutex);
            return OT_ERR_HDMI_MALLOC_FAILED;
        }
        (td_void)memset_s(tmp, sizeof(hdmi_callback_node), 0, sizeof(hdmi_callback_node));
        tmp->callback_func = *callback_func;
    } else {
        hdmi_err_trace("Callbackaddr is NULL!\n");
        hdmi_unlock(g_callback_mutex);
        return OT_ERR_HDMI_INVALID_CALLBACK;
    }
    list_add_tail(&(tmp->list), &(g_hdmi_callback_list[hdmi].list));
    hdmi_unlock(g_callback_mutex);
    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_REGISTER_CALLBACK, &hdmi);
    hdmi_unlock(g_hdmi_mutex);
    hdmi_check_failure_return(ret);

    return ret;
}

td_s32 mpi_hdmi_com_unreg_callback_func(ot_hdmi_id hdmi, const ot_hdmi_callback_func *callback_func)
{
    td_s32 ret;
    hdmi_callback_node *tmp = TD_NULL;
    struct list_head *pos = TD_NULL;
    struct list_head *q = TD_NULL;

    hdmi_check_id_return(hdmi);
    hdmi_check_null_return(callback_func, OT_ERR_HDMI_NULL_PTR);
    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);
    hdmi_unlock(g_hdmi_mutex);

    hdmi_check_lock_return(g_callback_mutex, OT_ERR_HDMI_MUTEX_LOCK_FAILED);
    list_for_each_safe(pos, q, &g_hdmi_callback_list[hdmi].list) {
        tmp = list_entry(pos, hdmi_callback_node, list);
        if (tmp != TD_NULL && callback_func->hdmi_event_callback != TD_NULL &&
            tmp->callback_func.hdmi_event_callback == callback_func->hdmi_event_callback) {
            list_del(pos);
            free(tmp);
            tmp = TD_NULL;
            ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_UNREGISTER_CALLBACK, &hdmi);
            hdmi_unlock(g_callback_mutex);
            hdmi_check_failure_return(ret);

            return ret;
        }
    }
    hdmi_unlock(g_callback_mutex);
    hdmi_err_trace("No this Callbackfunc\n");

    return OT_ERR_HDMI_CALLBACK_NOT_REGISTER;
}

td_s32 mpi_hdmi_com_set_hw_spec(ot_hdmi_id hdmi, const ot_hdmi_hw_spec *hw_spec)
{
#ifdef HDMI_DFX_SUPPORT
    td_s32 ret;
    drv_hdmi_hw_spec spec = {0};

    hdmi_check_id_return(hdmi);
    hdmi_check_null_return(hw_spec, OT_ERR_HDMI_NULL_PTR);

    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);
    spec.hdmi_id = hdmi;
    ret = memcpy_s(&spec.hw_spec, sizeof(hdmi_hw_spec), hw_spec, sizeof(ot_hdmi_hw_spec));
    if (ret != EOK) {
        hdmi_err_trace("memcpy_s fail.\n");
        hdmi_unlock(g_hdmi_mutex);
        return OT_ERR_HDMI_INVALID_PARA;
    }

    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_SET_HW_SPEC, &spec);
    if (ret != TD_SUCCESS) {
        hdmi_unlock(g_hdmi_mutex);
        hdmi_check_failure_return(ret);
        return ret;
    }
    hdmi_unlock(g_hdmi_mutex);

    return ret;
#else
    ot_unused(hdmi);
    ot_unused(hw_spec);
    return TD_SUCCESS;
#endif
}

td_s32 mpi_hdmi_com_get_hw_spec(ot_hdmi_id hdmi, ot_hdmi_hw_spec *hw_spec)
{
#ifdef HDMI_DFX_SUPPORT
    td_s32 ret;
    drv_hdmi_hw_spec spec = {0};

    hdmi_check_id_return(hdmi);
    hdmi_check_null_return(hw_spec, OT_ERR_HDMI_NULL_PTR);

    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);
    spec.hdmi_id = hdmi;
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_GET_HW_SPEC, &spec);
    if (ret != TD_SUCCESS) {
        hdmi_unlock(g_hdmi_mutex);
        hdmi_check_failure_return(ret);
        return ret;
    }
    ret = memcpy_s(hw_spec, sizeof(ot_hdmi_hw_spec), &spec.hw_spec, sizeof(hdmi_hw_spec));
    if (ret != EOK) {
        hdmi_err_trace("memcpy_s fail.\n");
        hdmi_unlock(g_hdmi_mutex);
        return OT_ERR_HDMI_INVALID_PARA;
    }

    hdmi_unlock(g_hdmi_mutex);

    return ret;
#else
    ot_unused(hdmi);
    ot_unused(hw_spec);
    return TD_SUCCESS;
#endif
}

td_s32 mpi_hdmi_com_set_attr(ot_hdmi_id hdmi, drv_hdmi_property *property)
{
    td_s32 ret;

    hdmi_check_id_return(hdmi);
    hdmi_check_null_return(property, OT_ERR_HDMI_NULL_PTR);

    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);
    property->hdmi_id = hdmi;
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_SET_ATTR, property);
    if (ret != TD_SUCCESS) {
        hdmi_unlock(g_hdmi_mutex);
        hdmi_check_failure_return(ret);
        return ret;
    }
    hdmi_unlock(g_hdmi_mutex);

    return ret;
}

td_s32 mpi_hdmi_com_get_attr(ot_hdmi_id hdmi, drv_hdmi_property *property)
{
    td_s32 ret;

    hdmi_check_id_return(hdmi);
    hdmi_check_null_return(property, OT_ERR_HDMI_NULL_PTR);

    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);
    property->hdmi_id = hdmi;
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_GET_ATTR, property);
    if (ret != TD_SUCCESS) {
        hdmi_unlock(g_hdmi_mutex);
        hdmi_check_failure_return(ret);
        return ret;
    }
    hdmi_unlock(g_hdmi_mutex);

    return ret;
}

td_s32 mpi_hdmi_com_set_avmute(ot_hdmi_id hdmi, td_bool enable_avmute)
{
#ifdef HDMI_DFX_SUPPORT
    td_s32 ret;
    drv_hdmi_avmute drv_avmute = {0};

    hdmi_check_id_return(hdmi);
    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);

    drv_avmute.hdmi_id = hdmi;
    drv_avmute.avmute = enable_avmute;
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_SET_AVMUTE, &drv_avmute);
    hdmi_unlock(g_hdmi_mutex);
    if (ret != TD_SUCCESS) {
        hdmi_check_failure_return(ret);
        return ret;
    }

    return TD_SUCCESS;

#else
    ot_unused(hdmi);
    ot_unused(enable_avmute);
    return TD_SUCCESS;
#endif
}

td_s32 mpi_hdmi_com_mod_param_set(ot_hdmi_id hdmi, const ot_hdmi_mod_param *user_mod_param)
{
#ifdef HDMI_DFX_SUPPORT
    td_s32 ret;
    drv_hdmi_mod_param mod_param = {0};

    hdmi_check_id_return(hdmi);
    hdmi_check_null_return(user_mod_param, OT_ERR_HDMI_NULL_PTR);

    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);
    mod_param.hdmi_id = hdmi;
#ifndef HDMI_PRODUCT_CHIP_1
    mod_param.emi_en = user_mod_param->emi_en;
    mod_param.trace_len = user_mod_param->trace_len;
#else
    hdmi_debug_trace("not support, set default.\n");
    mod_param.trace_len = OT_HDMI_TRACE_DEFAULT;
    mod_param.emi_en = TD_FALSE;
#endif
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_SET_MOD_PARAM, &mod_param);
    if (ret != TD_SUCCESS) {
        hdmi_unlock(g_hdmi_mutex);
        hdmi_check_failure_return(ret);
        return ret;
    }
    hdmi_unlock(g_hdmi_mutex);

    return ret;
#else
    ot_unused(hdmi);
    ot_unused(user_mod_param);
    return TD_SUCCESS;
#endif
}

td_s32 mpi_hdmi_com_mod_param_get(ot_hdmi_id hdmi, ot_hdmi_mod_param *user_mod_param)
{
#ifdef HDMI_DFX_SUPPORT
    td_s32 ret;
    drv_hdmi_mod_param mod_param = {0};

    hdmi_check_id_return(hdmi);
    hdmi_check_null_return(user_mod_param, OT_ERR_HDMI_NULL_PTR);

    hdmi_check_lock_return(g_hdmi_mutex, OT_ERR_HDMI_DEV_NOT_OPEN);
    hdmi_check_chn_open_return(hdmi, g_hdmi_mutex);
    mod_param.hdmi_id = hdmi;
    ret = (td_s32)ioctl(g_hdmi_dev_fd, CMD_HDMI_GET_MOD_PARAM, &mod_param);
    if (ret != TD_SUCCESS) {
        hdmi_unlock(g_hdmi_mutex);
        hdmi_check_failure_return(ret);
        return ret;
    }
    user_mod_param->emi_en = mod_param.emi_en;
    user_mod_param->trace_len = mod_param.trace_len;
    hdmi_unlock(g_hdmi_mutex);

    return ret;
#else
    ot_unused(hdmi);
    ot_unused(user_mod_param);
    return TD_SUCCESS;
#endif
}

hdmi_deep_color deep_color_user2drv(ot_hdmi_deep_color user_deep_color)
{
    hdmi_deep_color ret;

    switch (user_deep_color) {
        case OT_HDMI_DEEP_COLOR_24BIT:
            ret = HDMI_DEEP_COLOR_24BIT;
            break;
        case OT_HDMI_DEEP_COLOR_30BIT:
            ret = HDMI_DEEP_COLOR_30BIT;
            break;
        case OT_HDMI_DEEP_COLOR_36BIT:
            ret = HDMI_DEEP_COLOR_36BIT;
            break;
        default:
            ret = HDMI_DEEP_COLOR_BUTT;
            break;
    }

    return ret;
}

ot_hdmi_deep_color deep_color_drv2user(hdmi_deep_color kernel_deep_color)
{
    ot_hdmi_deep_color ret;

    switch (kernel_deep_color) {
        case HDMI_DEEP_COLOR_24BIT:
            ret = OT_HDMI_DEEP_COLOR_24BIT;
            break;
        case HDMI_DEEP_COLOR_30BIT:
            ret = OT_HDMI_DEEP_COLOR_30BIT;
            break;
        case HDMI_DEEP_COLOR_36BIT:
            ret = OT_HDMI_DEEP_COLOR_36BIT;
            break;
        case HDMI_DEEP_COLOR_OFF:
            ret = OT_HDMI_DEEP_COLOR_24BIT;
            break;
        default:
            ret = OT_HDMI_DEEP_COLOR_BUTT;
            break;
    }

    return ret;
}

