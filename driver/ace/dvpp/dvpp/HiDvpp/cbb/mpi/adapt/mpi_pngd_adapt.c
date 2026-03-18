/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021. All rights reserved.
* Description:define pngd adapt function
* Author: Hisilicon multimedia software group
* Create: 2021/07/22
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>

#include "HiDvppSysAdapter.h"

#include "mpi_pngd_adapt.h"
#ifdef BUILD_MPI_ADAPTER
#include "hi_mpi_redefine.h"
#endif
#include "hi_inner_pngd.h"
#include "mpi_sys.h"
#include "mpi_sys_adapt.h"
#include "mkp_pngd.h"
#include "pngd_drv.h"
#include "HiDvppPngdInternal.h"
#include "weak_extern.h"
#include "ascend_hal.h" // 驱动的头文件，用于异步模式发送任务完成事件，位于inc/driver
#include "hi_math.h"

/*lint -e569 -e650*/
hi_s32 g_pngd_fd[INTERNAL_PNGD_MAX_CHN_NUM] = { [0 ... INTERNAL_PNGD_MAX_CHN_NUM - 1] = -1 };
struct mpi_context g_pngd_chn_context[INTERNAL_PNGD_MAX_CHN_NUM];

static pthread_mutex_t s_pngd_mutex = PTHREAD_MUTEX_INITIALIZER;

#define PNGD_DEV_NAME_LEN 128
#define PNGD_ERROR_NUM_HEAD 0xA0408000

hi_s32 trans_pngd_chan_id_to_device_id(hi_s32 chan_id)
{
    return chan_id / ((hi_s32)PNGD_MAX_CHN_NUM_PER_DEVICE);
}

hi_s32 trans_pngd_chan_id_to_device_chan_id(hi_s32 chan_id)
{
    return chan_id % ((hi_s32)PNGD_MAX_CHN_NUM_PER_DEVICE);
}

void check_pngd_return_value_and_update(hi_s32 *ret)
{
    if ((*ret != HI_SUCCESS) && (((*ret) & (0xFFFFF000)) != PNGD_ERROR_NUM_HEAD)) {
        *ret = HI_ERR_PNGD_SYS_NOT_READY;
    }
}

hi_s32 hi_mpi_pngd_open(const hi_char *pathname, const hi_u32 len, const hi_s32 flags)
{
    hi_s32 fd = -1;
    hi_char *real_path = HI_NULL;

    if (len == 0U) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "len is 0 \n");
        return HI_FAILURE;
    }
    if (pathname == HI_NULL) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pathname is Null \n");
        return HI_FAILURE;
    }

    real_path = realpath(pathname, HI_NULL);
    if (real_path == HI_NULL) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "realpath fail \n");
        return HI_FAILURE;
    }

    /* code_dex: race condition: file system access  */
    fd = open(real_path, flags, 0);
    if (fd < 0) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "open %s fail, error:%s\n", real_path, strerror(errno));
        free(real_path);
        return HI_FAILURE;
    }

    free(real_path);
    return fd;
}

hi_s32 pngd_check_open(hi_pngd_chn chn_id)
{
    (void)pthread_mutex_lock(&s_pngd_mutex);
    if (g_pngd_fd[chn_id] < 0) {
        g_pngd_fd[chn_id] = hi_mpi_pngd_open("/dev/pngd", PNGD_DEV_NAME_LEN, O_RDWR);
        if (g_pngd_fd[chn_id] < 0) {
            (void)pthread_mutex_unlock(&s_pngd_mutex);
            HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "open /dev/pngd err. drv_pngd.ko may not insmod\n");
            return HI_ERR_PNGD_SYS_NOT_READY;
        }
    }
    (void)pthread_mutex_unlock(&s_pngd_mutex);

    return HI_SUCCESS;
}

hi_s32 pngd_check_chn(hi_pngd_chn *pngd_chn)
{
    hi_s32 check_ret = HI_SUCCESS;
    hi_u32 device_id = 0;
    check_ret = HI_MPI_SYS_GetPhyDeviceId(&device_id);
    if (check_ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "HI_MPI_SYS_GetPhyDeviceId ERROR. \n");
        return (hi_s32)HI_ERR_PNGD_SYS_NOT_READY;
    }
    hi_u32 pngd_chn_id = (*pngd_chn);
    if (device_id >= DEVICE_NUM) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " PNGD device_id: %d isn't in the range of [0, %d]!\n",
                 device_id, DEVICE_NUM);
        return HI_ERR_PNGD_INVALID_CHN_ID;
    }
    if (pngd_chn_id >= PNGD_MAX_CHN_NUM_PER_DEVICE) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " PNGD Channel: %d, Chn id %d isn't in the range of [0, %d]!\n",
                 (*pngd_chn), pngd_chn_id, (PNGD_MAX_CHN_NUM_PER_DEVICE - 1));
        return HI_ERR_PNGD_INVALID_CHN_ID;
    }
    (*pngd_chn) = pngd_chn_id + (device_id * PNGD_MAX_CHN_NUM_PER_DEVICE);

    check_ret = pngd_check_open(*pngd_chn);
    if (check_ret != HI_SUCCESS) {
        return check_ret;
    }

    return HI_SUCCESS;
}

hi_s32 pngd_check_stride(const hi_img_info *img_info, hi_pic_info *png_pic_info)
{
    const uint32_t ALIGN_IN_16_BYTE = 16;
    const uint32_t ALIGN_IN_128_BYTE = 128;
    const uint32_t PNG_RGB_TYPE_PIXEL_SIZE = 3; // RGB格式每个像素大小
    const uint32_t PNG_RGBA_TYPE_PIXEL_SIZE = 4; // RGBA格式每个像素大小
    uint32_t pixel_size = 0;
    uint32_t width_stride_in_16 = 0;
    uint32_t width_stride_in_128 = 0;
    uint32_t height_stride_in_128 = 0;

    if (png_pic_info->picture_format == PIXEL_FORMAT_RGB_888) {
        pixel_size = PNG_RGB_TYPE_PIXEL_SIZE;
    } else if (png_pic_info->picture_format == PIXEL_FORMAT_RGBA_8888) {
        pixel_size = PNG_RGBA_TYPE_PIXEL_SIZE;
    } else {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " hi_pixel_format:(%u) out the range\n", png_pic_info->picture_format);
        return HI_ERR_PNGD_ILLEGAL_PARAM;
    }

    width_stride_in_16 = ALIGN_UP(img_info->u32Width, ALIGN_IN_16_BYTE) * pixel_size;
    width_stride_in_128 = ALIGN_UP(img_info->u32Width, ALIGN_IN_128_BYTE)  * pixel_size;
    height_stride_in_128 = ALIGN_UP(img_info->u32Height, ALIGN_IN_128_BYTE);

    if ((png_pic_info->picture_width_stride == 0) && (png_pic_info->picture_height_stride == 0)) {
        png_pic_info->picture_width_stride = width_stride_in_128;
        png_pic_info->picture_height_stride = ALIGN_UP(img_info->u32Height, ALIGN_IN_16_BYTE);
        return HI_SUCCESS;
    }

    if ((png_pic_info->picture_width_stride != (img_info->u32Width * pixel_size)) &&
        (png_pic_info->picture_width_stride != width_stride_in_16) &&
        (png_pic_info->picture_width_stride != width_stride_in_128)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " width_stride in pic format:(%u) invalid. now is %u\n",
            png_pic_info->picture_format, png_pic_info->picture_width_stride);
        return HI_ERR_PNGD_ILLEGAL_PARAM;
    }

    if ((png_pic_info->picture_height_stride < img_info->u32Height) ||
        (png_pic_info->picture_height_stride > height_stride_in_128)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " height_stride:%u. out the range of [%u, %u]\n",
            png_pic_info->picture_height_stride, img_info->u32Height, height_stride_in_128);
        return HI_ERR_PNGD_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

hi_s32 hi_mpi_pngd_check_chn_is_destory(hi_pngd_chn chn_id)
{
    hi_s32 ret = HI_SUCCESS;
    chn_manager* pngd_chn_manager = get_pngd_chn_manager(chn_id);

    (void)pthread_mutex_lock(&pngd_chn_manager->chn_lock);
    if (pngd_chn_manager->chn_state == CHN_STATE_DESTROYED) { // 判断通道是否处于销毁状态
        pngd_chn_manager->chn_state = CHN_STATE_CREATING; // 修改通道状态为正在创建,互斥管理
        ret = HI_SUCCESS;
    } else {
        ret = HI_FAILURE;
    }
    (void)pthread_mutex_unlock(&pngd_chn_manager->chn_lock);

    return ret;
}

hi_s32 hi_mpi_pngd_check_chn_is_created(hi_pngd_chn chn_id)
{
    hi_s32 ret = HI_SUCCESS;
    chn_manager* pngd_chn_manager = get_pngd_chn_manager(chn_id);

    (void)pthread_mutex_lock(&pngd_chn_manager->chn_lock);
    if (pngd_chn_manager->chn_state == CHN_STATE_CREATED) { // 判断通道是否处于创建状态
        ret = HI_SUCCESS;
    } else {
        ret = HI_ERR_PNGD_UNEXIST;
    }
    (void)pthread_mutex_unlock(&pngd_chn_manager->chn_lock);

    return ret;
}

hi_void hi_mpi_pngd_set_chn_state_and_kernel_chn_id(hi_pngd_chn chn_id,
    dvpp_chn_state chn_state, hi_pngd_chn kernel_chn_id)
{
    chn_manager* pngd_chn_manager = get_pngd_chn_manager(chn_id);
    (void)pthread_mutex_lock(&pngd_chn_manager->chn_lock);

    pngd_chn_manager->chn_state = chn_state;
    pngd_chn_manager->kernel_chn_id = kernel_chn_id;

    (void)pthread_mutex_unlock(&pngd_chn_manager->chn_lock);
}

hi_s32 hi_mpi_pngd_get_user_unused_chn(hi_pngd_chn *user_chn_id)
{
    hi_u32 device_id = 0;
    hi_s32 ret = HI_SUCCESS;
    hi_s32 start_id = 0;
    hi_s32 end_id = 0;
    hi_s32 chn_id = 0;
    chn_manager* pngd_chn_manager = NULL;

    ret = HI_MPI_SYS_GetPhyDeviceId(&device_id);
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d, pngd get device id failed\n", hi_mpi_sys_get_tgid());
        return (hi_s32)HI_ERR_PNGD_SYS_NOT_READY;
    }

    if (device_id >= DEVICE_NUM) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d, device id[%u] is illegal\n", hi_mpi_sys_get_tgid(), device_id);
        return HI_ERR_PNGD_ILLEGAL_PARAM;
    }

    start_id = device_id * PNGD_MAX_CHN_NUM_PER_DEVICE;
    end_id = start_id + PNGD_MAX_CHN_NUM_PER_DEVICE;

    for (chn_id = start_id; chn_id < end_id; chn_id++) {
        pngd_chn_manager = get_pngd_chn_manager(chn_id);
        (void)pthread_mutex_lock(&pngd_chn_manager->chn_lock);
        if (pngd_chn_manager->chn_state == CHN_STATE_DESTROYED) {
            pngd_chn_manager->chn_state = CHN_STATE_CREATING;
            (void)pthread_mutex_unlock(&pngd_chn_manager->chn_lock);
            break;
        }
        (void)pthread_mutex_unlock(&pngd_chn_manager->chn_lock);
    }

    if (chn_id >= end_id) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d, pngd chnl is full.\n", hi_mpi_sys_get_tgid());
        return HI_ERR_PNGD_NOT_PERM;
    }

    *user_chn_id = chn_id;
    return HI_SUCCESS;
}

hi_s32 hi_mpi_pngd_create_unused_chn(hi_u32 device_id, hi_pngd_chn user_chn_id,
    hi_pngd_chn *kernel_chn_id, pngd_create_info *create_info)
{
    hi_s32 start_chn_id = device_id * PNGD_MAX_CHN_NUM_PER_DEVICE;
    hi_s32 end_chn_id = start_chn_id + PNGD_MAX_CHN_NUM_PER_DEVICE;
    hi_s32 i = 0;
    hi_s32 ret = HI_SUCCESS;
    hi_u32 logic_dev_id = 0; // 逻辑device id, 在910B和310B算力切分场景下,需要通过逻辑device id转物理device id
    hi_pngd_chn external_user_chn_id = user_chn_id % PNGD_MAX_CHN_NUM_PER_DEVICE;

    ret = pngd_check_open(user_chn_id); // 先用用户传入的通道号尝试创建
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = HI_MPI_SYS_GetDeviceId(&logic_dev_id); // 获取逻辑device id
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "GetDeviceId Fail\n");
        return HI_ERR_PNGD_SYS_ERROR;
    }

    create_info->chan_id = user_chn_id;
    create_info->user_chan_id = external_user_chn_id;
    create_info->logic_dev_id = logic_dev_id;
    ret = ioctl(g_pngd_fd[user_chn_id], PNGD_CHN_CREATE_CTRL, create_info);
    if (ret == HI_SUCCESS) {
        *kernel_chn_id = user_chn_id;
        return HI_SUCCESS;
    } else if (ret != (hi_s32)HI_ERR_PNGD_EXIST) { // 如果由于参数检查等其他错误，则直接返回，不再继续尝试其他通道号。
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " PNGD Create Channel Fail!\n");
        return ret;
    }

    // 轮询所有通道号尝试获取
    for (i = start_chn_id; i < end_chn_id; i++) {
        create_info->chan_id = i;
        create_info->user_chan_id = external_user_chn_id;
        ret = ioctl(g_pngd_fd[user_chn_id], PNGD_CHN_CREATE_CTRL, create_info);
        if (ret == HI_SUCCESS) {
            *kernel_chn_id = i;
            HI_TRACE(HI_DBG_INFO, HI_ID_PNGD, "pid %d, device id %u, user chn id %d create pngd chn success!\n",
                hi_mpi_sys_get_tgid(), device_id, external_user_chn_id);
            break;
        } else if (ret != (hi_s32)HI_ERR_PNGD_EXIST) { // 如果由于参数检查等其他错误，则直接返回，不再继续尝试其他通道号。
            HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " PNGD Create Channel Fail!\n");
            return ret;
        }
    }

    if (i >= end_chn_id) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d, device id %u, user chn id %d, pngd chnl is full",
            hi_mpi_sys_get_tgid(), device_id, external_user_chn_id);
        return HI_ERR_PNGD_NOT_PERM; //lint !e569
    }
    return HI_SUCCESS;
}
// user_set_chn_id记录用户传入的通道号，范围为0~255
// chn_id 根据device id转换为内部用户态通道号，范围为0~511
// kernel_chn_id 内核态创建成功的通道号，范围为0~511
hi_s32 hi_mpi_pngd_create_chn(hi_pngd_chn chn_id, const hi_pngd_chn_attr *attr, hi_bool is_acl)
{
    hi_s32 ret = HI_SUCCESS;
    hi_pngd_chn user_set_chn_id = chn_id; // 记录用户传入的通道号
    hi_u32 device_id = 0;
    hi_vdec_chn kernel_chn_id = 0;
    pngd_create_info create_info = {0};
    ret = pngd_check_chn(&chn_id);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    if (attr == NULL) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "attr is NULL pointer\n");
        return HI_ERR_PNGD_NULL_PTR;
    }

    create_info.is_acl = is_acl;
    create_info.chan_id = chn_id;
    ret = memcpy_s(&create_info.attr, sizeof(hi_pngd_chn_attr), attr, sizeof(hi_pngd_chn_attr));
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "memcpy_s err. \n");
        return HI_ERR_PNGD_SYS_ERROR; //lint !e569
    }

    device_id = chn_id / PNGD_MAX_CHN_NUM_PER_DEVICE;
    ret = hi_mpi_pngd_check_chn_is_destory(chn_id); // 判断通道是否处于销毁状态(0)
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d, device id %u, user chn id %d is exist\n",
            hi_mpi_sys_get_tgid(), device_id, user_set_chn_id);
        return HI_ERR_PNGD_EXIST;
    }

    ret = hi_mpi_pngd_create_unused_chn(device_id, chn_id, &kernel_chn_id, &create_info);
    if (ret != HI_SUCCESS) {
        check_pngd_return_value_and_update(&ret);
        HI_TRACE(HI_DBG_ERR, HI_ID_VDEC, "pid %d, device id %u, user chn id %d, pngd create chn fail, ret = %x",
            hi_mpi_sys_get_tgid(), device_id, user_set_chn_id, ret);
        // 修改通道状态和内核通道号为初始化状态(-1)
        hi_mpi_pngd_set_chn_state_and_kernel_chn_id(chn_id, CHN_STATE_DESTROYED, -1);
        return ret;
    }

    g_pngd_chn_context[chn_id].is_acl = is_acl; // 记录当前通道是否为acl通道
    hi_mpi_pngd_set_chn_state_and_kernel_chn_id(chn_id, CHN_STATE_CREATED, kernel_chn_id); // 修改通道状态和内核通道号
    HI_TRACE(HI_DBG_INFO, HI_ID_PNGD, "pid %d, device id %u, user chn id %d, kernel chn id %d, create success\n",
        hi_mpi_sys_get_tgid(), device_id, user_set_chn_id, kernel_chn_id);

    return ret;
}

static hi_s32 hi_mpi_pngd_create_chn_ex_check(hi_pngd_chn *chn_id, const hi_pngd_chn_attr_ex *attr_ex,
    hi_u32 *device_id, pngd_create_info *create_info)
{
    hi_s32 ret = HI_SUCCESS;
    if ((chn_id == NULL) || (attr_ex == NULL)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "chn_id or attr_ex is NULL pointer\n");
        return HI_ERR_PNGD_NULL_PTR;
    }

    ret = HI_MPI_SYS_GetPhyDeviceId(device_id);
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "HI_MPI_SYS_GetPhyDeviceId ERROR.\n");
        return (hi_s32)HI_ERR_PNGD_SYS_NOT_READY;
    }

    create_info->is_acl = HI_TRUE;
    ret = memcpy_s(&create_info->attr, sizeof(hi_pngd_chn_attr), &attr_ex->chanl_attr, sizeof(hi_pngd_chn_attr));
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "memcpy_s err. \n");
        return HI_ERR_PNGD_SYS_ERROR;
    }

    return HI_SUCCESS;
}

// chn_id 返回给用户的通道号，范围为0~255
// user_chn_id 内部用户态通道号，范围为0~511
// kernel_chn_id 内核态创建成功的通道号，范围为0~511
hi_s32 hi_mpi_pngd_create_chn_ex(hi_pngd_chn *chn_id, const hi_pngd_chn_attr_ex *attr_ex)
{
    hi_s32 ret = HI_SUCCESS;
    pngd_acl_info acl_info;
    pngd_create_info create_info;
    hi_u32 device_id = 0;
    hi_vdec_chn kernel_chn_id = 0;
    hi_vdec_chn user_chn_id = 0; // 内部用户态通道id， 0~511

    ret = hi_mpi_pngd_create_chn_ex_check(chn_id, attr_ex, &device_id, &create_info);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = hi_mpi_pngd_get_user_unused_chn(&user_chn_id); // 先查找一个空闲的用户态通道号0~511
    if (ret != HI_SUCCESS) {
        return ret;
    }

    *chn_id = user_chn_id % PNGD_MAX_CHN_NUM_PER_DEVICE;

    ret = hi_mpi_pngd_create_unused_chn(device_id, user_chn_id, &kernel_chn_id, &create_info); // 查找一个内核态空闲的通道号
    if (ret != HI_SUCCESS) {
        check_pngd_return_value_and_update(&ret);
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d, device id %u, pngd create chn ex fail, ret = %lx",
            hi_mpi_sys_get_tgid(), device_id, ret);
        // 修改通道状态和内核通道号为初始化状态(-1)
        hi_mpi_pngd_set_chn_state_and_kernel_chn_id(user_chn_id, CHN_STATE_DESTROYED, -1);
        return ret;
    }

    acl_info.pid = hi_mpi_sys_get_tgid();
    acl_info.is_himpi = HI_FALSE;
    acl_info.user_data = attr_ex->user_data;
    ret = ioctl(g_pngd_fd[user_chn_id], PNGD_CHN_SET_ACL_CTRL, &acl_info);
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "create channel %d ioctl SET_ACL failed.\n", user_chn_id);
        (void)hi_mpi_pngd_destroy_chn(*chn_id); // 使用外部0~255的通道号
        check_pngd_return_value_and_update(&ret);
        // 修改通道状态和内核通道号为初始化状态(-1)
        hi_mpi_pngd_set_chn_state_and_kernel_chn_id(user_chn_id, CHN_STATE_DESTROYED, -1);
        return ret;
    }

    g_pngd_chn_context[user_chn_id].is_acl = HI_TRUE; // 记录当前通道是为acl通道
    hi_mpi_pngd_set_chn_state_and_kernel_chn_id(user_chn_id, CHN_STATE_CREATED, kernel_chn_id); // 修改通道状态和内核通道号
    HI_TRACE(HI_DBG_INFO, HI_ID_PNGD, "pid %d, device id %u, user chn id %d, kernel chn id %d, create success\n",
        hi_mpi_sys_get_tgid(), device_id, *chn_id, kernel_chn_id);

    return ret;
}

hi_s32 hi_mpi_pngd_create_chn2(hi_pngd_chn chn_id, const hi_pngd_chn_attr *attr, hi_u32 flag)
{
    hi_s32 ret = HI_SUCCESS;
    pngd_acl_info acl_info;
    hi_u32 device_id = 0;
    hi_u32 ioctl_chn_id = 0;
    if (attr == NULL) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "attr is NULL pointer\n");
        return HI_ERR_PNGD_NULL_PTR;
    }

    if (flag) {
        // reserved for feature use
        return HI_ERR_PNGD_ILLEGAL_PARAM;
    }

    ret = HI_MPI_SYS_GetPhyDeviceId(&device_id);
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "get device id fail\n");
        return (hi_s32)HI_ERR_PNGD_SYS_NOT_READY;
    }

    ret = hi_mpi_pngd_create_chn(chn_id, attr, HI_TRUE);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    acl_info.pid = hi_mpi_sys_get_tgid();
    acl_info.is_himpi = HI_TRUE;
    acl_info.user_data = HI_NULL;

    ioctl_chn_id = chn_id + (device_id * PNGD_MAX_CHN_NUM_PER_DEVICE);

    ret = ioctl(g_pngd_fd[ioctl_chn_id], PNGD_CHN_SET_ACL_CTRL, &acl_info);
    if (ret != HI_SUCCESS) {
        (void)hi_mpi_pngd_destroy_chn(chn_id);
        check_pngd_return_value_and_update(&ret);
        // 修改通道状态和内核通道号为初始化状态(-1)
        hi_mpi_pngd_set_chn_state_and_kernel_chn_id(ioctl_chn_id, CHN_STATE_DESTROYED, -1);
        return ret;
    }

    return HI_SUCCESS;
}

hi_s32 hi_mpi_pngd_destroy_chn(hi_pngd_chn chn_id)
{
    hi_s32 ret = HI_SUCCESS;
    hi_vdec_chn user_set_chn_id = chn_id; // 记录用户传入的通道号
    chn_manager* pngd_chn_manager = NULL;
    hi_u32 device_id = 0;
    ret = pngd_check_chn(&chn_id);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    device_id = chn_id / PNGD_MAX_CHN_NUM_PER_DEVICE;
    ret = hi_mpi_pngd_check_chn_is_created(chn_id);
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d, device id %u, user chn id %d is unexist\n",
            hi_mpi_sys_get_tgid(), device_id, user_set_chn_id);
        return ret;
    }

    ret = ioctl(g_pngd_fd[chn_id], PNGD_CHN_DESTROY_CTRL);
    if (ret == HI_ERR_PNGD_UNEXIST) {
        check_pngd_return_value_and_update(&ret);
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d usr chn %d device %d chn %d has not been created\n",
            hi_mpi_sys_get_tgid(), user_set_chn_id,
            trans_pngd_chan_id_to_device_id(chn_id), trans_pngd_chan_id_to_device_chan_id(chn_id));
    }

    if (ret == HI_SUCCESS) {
        pngd_chn_manager = get_pngd_chn_manager(chn_id);
        HI_TRACE(HI_DBG_INFO, HI_ID_PNGD, "pid %d, device id %u, user chn id %d, kernel chn id %d, destroy success\n",
            hi_mpi_sys_get_tgid(), device_id, user_set_chn_id, pngd_chn_manager->kernel_chn_id);
        // 修改通道状态和内核通道号 -1
        hi_mpi_pngd_set_chn_state_and_kernel_chn_id(chn_id, CHN_STATE_DESTROYED, -1);
    }
    return ret;
}

// ACL和host himpi场景校验用户申请的输入/输出buffer是否有效
hi_s32 check_pngd_user_buffer_validity(hi_u64 *addr, hi_u32 len)
{
    hi_s32 ret = HI_SUCCESS;
    hi_u32 device_id = 0;
    struct MemInfo info;
    hi_u64 tmp_addr = 0;
    hi_u64* addr_info[2]; // 地址首尾各2个地址，总共2个地址

    CHECK_DO_SOMETHING(!halMemGetInfoEx, HI_TRACE(HI_DBG_WARN, HI_ID_PNGD, "halMemGetInfoEx is not exit.\n");
        return HI_SUCCESS);

    ret = HI_MPI_SYS_GetPhyDeviceId(&device_id);
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "HI_MPI_SYS_GetPhyDeviceId ERROR. \n");
        return HI_ERR_PNGD_SYS_NOT_READY;
    }

    addr_info[0] = addr; // 首地址
    tmp_addr = (hi_u64)(uintptr_t)addr + (hi_u64)(len - 1); //lint !e507
    addr_info[1] = (hi_u64*)((uintptr_t)tmp_addr); // 尾地址

    info.addr_info.cnt = sizeof(addr_info) / sizeof(addr_info[0]);
    HI_TRACE(HI_DBG_INFO, HI_ID_PNGD, "info.addr_info.cnt:%u.\n", info.addr_info.cnt);
    info.addr_info.addr = addr_info;
#ifdef DVPP_ML
    info.addr_info.mem_type = MEM_DEV_TYPE | MEM_DVPP_TYPE;
#else
    info.addr_info.mem_type = MEM_DVPP_TYPE;
#endif
    info.addr_info.flag = 0;
    ret = halMemGetInfoEx(device_id, MEM_INFO_TYPE_ADDR_CHECK, &info);
    if (ret != DRV_ERROR_NONE) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "addr:0x****%04x****%04x check failed ret 0x%x, please check: "
            "1. use hi_mpi_dvpp_malloc or aclDvppMalloc to alloc dvpp memory; "
            "2. make sure mem actual size should be:%u\n",
            ADDR_32_48_BITS((hi_u64)addr), ADDR_0_16_BITS((hi_u64)addr), ret, len);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

hi_s32 pngd_check_send_param_valid_check(hi_pngd_chn chn_id, const hi_img_stream *stream, hi_pic_info *png_pic_info)
{
    hi_s32 ret = HI_SUCCESS;
    hi_s32 device_idx = trans_pngd_chan_id_to_device_id(chn_id);
    hi_s32 chn_id_in_device = trans_pngd_chan_id_to_device_chan_id(chn_id);
    hi_u64* check_addr = NULL;

    if (stream->type != PT_PNG) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "device %u chn %u, stream->type:%u isn't HI_PT_PNG!",
            device_idx, chn_id_in_device, stream->type);
        return HI_ERR_PNGD_ILLEGAL_PARAM;
    }

    // 有效性检查
    if (g_pngd_chn_context[chn_id].is_acl == HI_TRUE) {
        check_addr = (hi_u64 *)(hi_uintptr_t)stream->addr; //lint !e507
        ret = check_pngd_user_buffer_validity(check_addr, stream->len);
        if (ret != HI_SUCCESS) {
            HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "device %u chn %u, input buffer is invalid. len %u.\n",
                device_idx, chn_id_in_device, stream->len);
            return HI_ERR_PNGD_BAD_ADDR;
        }

        check_addr = (hi_u64*)(hi_uintptr_t)png_pic_info->picture_address; //lint !e507
        ret = check_pngd_user_buffer_validity(check_addr, png_pic_info->picture_buffer_size);
        if (ret != HI_SUCCESS) {
            HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "device %u chn %u, output buffer is invalid. len %u.\n",
                device_idx, chn_id_in_device, png_pic_info->picture_buffer_size);
            return HI_ERR_PNGD_BAD_ADDR;
        }
    }

    return HI_SUCCESS;
}

hi_s32 pngd_check_send_param_valid(hi_pngd_chn chn_id, const hi_img_stream *stream, hi_pic_info *png_pic_info)
{
    hi_s32 ret = HI_SUCCESS;
    hi_img_info img_info = {0};

    ret = pngd_check_send_param_valid_check(chn_id, stream, png_pic_info);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = pngd_soft_parse_stream(stream, &img_info);
    if (ret != HI_SUCCESS) {
        ret = HI_DEF_ERR(HI_ID_PNGD, EN_ERR_LEVEL_ERROR, ret);
    }

    if (png_pic_info->picture_format == PIXEL_FORMAT_UNKNOWN) {
        if ((img_info.pngPixelFormat == PNG_COLOR_FORMAT_GRAY) ||
            (img_info.pngPixelFormat == PNG_COLOR_FORMAT_RGB)) {
            png_pic_info->picture_format = PIXEL_FORMAT_RGB_888;
        } else if ((img_info.pngPixelFormat == PNG_COLOR_FORMAT_AGRAY) ||
                   (img_info.pngPixelFormat == PNG_COLOR_FORMAT_ARGB)) {
            png_pic_info->picture_format = PIXEL_FORMAT_RGBA_8888;
        } else {
            HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " png pixel format:(%u) out the range\n", img_info.pngPixelFormat);
            return HI_ERR_PNGD_ILLEGAL_PARAM;
        }
    }

    ret = pngd_check_stride(&img_info, png_pic_info);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    if (png_pic_info->picture_buffer_size <
        (png_pic_info->picture_width_stride * png_pic_info->picture_height_stride)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " picture_buf_size:%u is too small. it must bigger than %u\n",
            png_pic_info->picture_buffer_size,
            (png_pic_info->picture_width_stride * png_pic_info->picture_height_stride));
        return HI_ERR_PNGD_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static hi_s32 hi_mpi_pngd_send_stream_check_null(const hi_img_stream *stream, hi_pic_info *png_pic_info)
{
    if ((stream == NULL) || (png_pic_info == NULL)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "stream or png_pic_info is NULL pointer\n");
        return HI_ERR_PNGD_NULL_PTR;
    }
    if ((stream->addr == NULL) || (png_pic_info->picture_address == NULL)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "stream->addr or png_pic_info->picture_address is NULL pointer\n");
        return HI_ERR_PNGD_NULL_PTR;
    }

    return HI_SUCCESS;
}

hi_s32 hi_mpi_pngd_send_stream(hi_pngd_chn chn_id, const hi_img_stream *stream, hi_pic_info *png_pic_info,
    hi_s32 milli_sec)
{
    hi_s32 ret = HI_SUCCESS;
    hi_u32 device_idx = 0;
    hi_u32 chn_id_in_device = 0;
    hi_vdec_chn user_set_chn_id = chn_id; // 记录用户传入的通道号
    pngd_send_stream_info send_stream;
    ret = pngd_check_chn(&chn_id);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    if (hi_mpi_pngd_send_stream_check_null(stream, png_pic_info) != HI_SUCCESS) {
        return HI_ERR_PNGD_NULL_PTR;
    }

    send_stream.milli_sec = milli_sec;
    send_stream.pid = hi_mpi_sys_get_tgid();

    device_idx = trans_pngd_chan_id_to_device_id(chn_id);
    chn_id_in_device = trans_pngd_chan_id_to_device_chan_id(chn_id);

    ret = hi_mpi_pngd_check_chn_is_created(chn_id);
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d, device id %u, user chn id %d is unexist\n",
            hi_mpi_sys_get_tgid(), device_idx, user_set_chn_id);
        return ret;
    }

    ret = pngd_check_send_param_valid(chn_id, stream, png_pic_info);
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %u usr chn %d device %u chn %u, check send param invalid. ret:%#x\n",
            hi_mpi_sys_get_tgid(), user_set_chn_id, device_idx, chn_id_in_device, ret);
        return ret;
    }

    ret = memcpy_s(&send_stream.stream, sizeof(hi_img_stream), stream, sizeof(hi_img_stream));
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d usr chn %d device %u chn %u, memcpy_s hi_img_stream err.\n",
            hi_mpi_sys_get_tgid(), user_set_chn_id, device_idx, chn_id_in_device);
        return HI_ERR_PNGD_SYS_ERROR;
    }
    ret = memcpy_s(&send_stream.png_pic_info, sizeof(hi_pic_info), png_pic_info, sizeof(hi_pic_info));
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d usr chn %d device %u chn %u, memcpy_s hi_pic_info err. \n",
            hi_mpi_sys_get_tgid(), user_set_chn_id, device_idx, chn_id_in_device);
        return HI_ERR_PNGD_SYS_ERROR;
    }

    ret = ioctl(g_pngd_fd[chn_id], PNGD_CHN_SENDSTREAM_CTRL, &send_stream);
    if (ret == HI_ERR_PNGD_UNEXIST) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d usr chn %d device %d chn %d has not been created\n",
            hi_mpi_sys_get_tgid(), user_set_chn_id, device_idx, chn_id_in_device);
    }
    check_pngd_return_value_and_update(&ret);

    return ret;
}

hi_s32 hi_mpi_pngd_get_image_data(hi_pngd_chn chn_id, hi_pic_info *png_pic_info,
                                  hi_img_stream *stream, hi_s32 milli_sec)
{
    hi_s32 ret = HI_SUCCESS;
    hi_s32 soft_ret = 0;
    hi_u32 device_idx = 0;
    hi_u32 chn_id_in_device = 0;
    pngd_get_image_info get_image = {0};
    hi_vdec_chn user_set_chn_id = chn_id; // 记录用户传入的通道号

    ret = pngd_check_chn(&chn_id);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    device_idx = trans_pngd_chan_id_to_device_id(chn_id);
    chn_id_in_device = trans_pngd_chan_id_to_device_chan_id(chn_id);
    if ((stream == NULL) || (png_pic_info == NULL)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d usr chn %d device %d chn %d stream or png_pic_info is NULL pointer\n",
            hi_mpi_sys_get_tgid(), user_set_chn_id, device_idx, chn_id_in_device);
        return HI_ERR_PNGD_NULL_PTR;
    }

    get_image.png_pic_info = png_pic_info;
    get_image.milli_sec = milli_sec;
    get_image.stream = stream;

    ret = hi_mpi_pngd_check_chn_is_created(chn_id);
    if (ret != HI_SUCCESS) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d, device id %u, user chn id %d is unexist\n",
            hi_mpi_sys_get_tgid(), device_idx, user_set_chn_id);
        return ret;
    }

    ret = ioctl((g_pngd_fd[chn_id]), (PNGD_CHN_GETIMAGE_CTRL), (&get_image));
    if (ret == HI_SUCCESS) {
        soft_ret = pngd_soft_decode(png_pic_info, stream);
        if (soft_ret != HI_SUCCESS) {
            ret = HI_DEF_ERR(HI_ID_PNGD, EN_ERR_LEVEL_ERROR, soft_ret);
        }
    }

    if (ret == HI_ERR_PNGD_UNEXIST) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "pid %d usr chn %d device %d chn %d has not been created\n",
            hi_mpi_sys_get_tgid(), user_set_chn_id,
            trans_pngd_chan_id_to_device_id(chn_id), trans_pngd_chan_id_to_device_chan_id(chn_id));
    }
    check_pngd_return_value_and_update(&ret);
    return ret;
}

hi_s32 hi_mpi_png_get_image_info(const hi_img_stream *png_stream, hi_img_info *img_info)
{
    hi_s32 ret = HI_SUCCESS;
    if ((png_stream == NULL) || (img_info == NULL)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "png_stream or img_info is NULL pointer\n");
        return HI_ERR_PNGD_NULL_PTR;
    }
    if (png_stream->addr == NULL) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "png_stream->addr is NULL pointer\n");
        return HI_ERR_PNGD_NULL_PTR;
    }

    CHECK_DO_SOMETHING((png_stream->type != PT_PNG),
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " png_stream->type:%u is't PT_PNG!\n"); return HI_ERR_PNGD_ILLEGAL_PARAM);

    ret = pngd_soft_parse_stream(png_stream, img_info);
    if (ret != HI_SUCCESS) {
        ret = HI_DEF_ERR(HI_ID_PNGD, EN_ERR_LEVEL_ERROR, ret);
    }
    return ret;
}
/*lint +e569 +e650*/
