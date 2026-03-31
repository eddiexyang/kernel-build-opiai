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

#include "dev_ext.h"
#include "pngd.h"
#include "pngd_intf.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif // end of #ifdef __cplusplus

hi_s32 pngd_check_chn_valid(hi_s32 chn_id)
{
    if (chn_id < 0) {
        PNGD_ERR_TRACE("pngd chnl:%d has not been created \n", chn_id);
        return HI_ERR_PNGD_UNEXIST;
    }
    if (chn_id >= (g_device_num * PNGD_MAX_CHN_NUM_PER_DEVICE)) {
        PNGD_ERR_TRACE("device id %d is out of range[0, %d) \n",
            chn_id / PNGD_MAX_CHN_NUM_PER_DEVICE, g_device_num);
        return HI_ERR_PNGD_INVALID_CHN_ID;
    }
    return HI_SUCCESS;
}

hi_s32 pngd_create_chn(hi_uintptr_t arg, hi_void *private_data)
{
    hi_s32 chn_id;
    hi_s32 ret = HI_SUCCESS;
    pngd_create_info *create_info = (pngd_create_info *)arg;
    if (create_info == NULL) {
        return HI_ERR_PNGD_NULL_PTR;
    }

    if ((UMAP_GET_CHN(private_data)) != -1) {
        return HI_ERR_PNGD_EXIST;
    }
    ret = pngd_check_module_func(PT_PNG);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    chn_id = create_info->chan_id;
    ret = pngd_check_chn_valid(chn_id);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = pngd_create(chn_id, create_info);
    if (ret == HI_SUCCESS) {
        UMAP_SET_CHN(private_data, chn_id); // 通道和fd绑定
    }
    return ret;
}

hi_s32 pngd_destory_chn(hi_uintptr_t arg, hi_void *private_data)
{
    hi_s32 chn_id = 0;
    hi_s32 ret = HI_SUCCESS;
    chn_id = UMAP_GET_CHN(private_data);

    ret = pngd_destroy(chn_id);
    if (ret == HI_SUCCESS) {
        UMAP_SET_CHN(private_data, -1);
    }

    return ret;
}

hi_s32 pngd_send_chn_stream(hi_uintptr_t arg, hi_void *private_data)
{
    hi_s32 chn_id = 0;
    hi_s32 ret = 0;
    hi_s32 device_id = 0;
    hi_s32 device_chan_id = 0;
    pngd_send_stream_info *send_stream = (pngd_send_stream_info *)arg;
    chn_id = UMAP_GET_CHN(private_data);
    ret = pngd_check_chn_valid(chn_id);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    device_id = trans_pngd_chan_id_to_device_id(chn_id);
    device_chan_id = trans_pngd_chan_id_to_device_chan_id(chn_id);

    if (send_stream == HI_NULL) {
        PNGD_ERR_TRACE("pid %d usr chn %d device %d chn %d stream param addr is null!\n",
            current->tgid, trans_pngd_chan_id_to_usr_chn_id(chn_id), device_id, device_chan_id);
        return HI_ERR_PNGD_NULL_PTR;
    }

    ret = pngd_send_stream(chn_id, &(send_stream->stream), send_stream->png_pic_info,
                           send_stream->pid, send_stream->milli_sec);

    return ret;
}

hi_s32 pngd_get_chn_image(hi_uintptr_t arg, hi_void *private_data)
{
    hi_s32 chn_id = 0;
    hi_s32 ret = HI_FAILURE;
    pngd_get_image_info *get_image = (pngd_get_image_info *)arg;
    hi_pic_info png_pic_info;
    hi_img_stream stream;

    (void)memset_s(&png_pic_info, sizeof(png_pic_info), 0, sizeof(png_pic_info));
    (void)memset_s(&stream, sizeof(stream), 0, sizeof(stream));

    chn_id = UMAP_GET_CHN(private_data);
    ret = pngd_check_chn_valid(chn_id);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    if (get_image != HI_NULL) {
        if (get_image->milli_sec < -1) {
            PNGD_ERR_TRACE("pid %d usr chn %d device %d chn %d milli_sec %d is illegal!\n",
                current->tgid, trans_pngd_chan_id_to_usr_chn_id(chn_id), trans_pngd_chan_id_to_device_id(chn_id),
                trans_pngd_chan_id_to_device_chan_id(chn_id), get_image->milli_sec);
            return HI_ERR_PNGD_ILLEGAL_PARAM;
        }
        ret = pngd_get_image(chn_id, &png_pic_info, &stream, get_image->milli_sec);
        if (ret == HI_SUCCESS) {
            if (osal_copy_to_user(get_image->png_pic_info, (void *)&png_pic_info, sizeof(hi_pic_info))) {
                ret = HI_ERR_PNGD_ILLEGAL_PARAM;
            }

            if (osal_copy_to_user(get_image->stream, (void *)&stream, sizeof(hi_img_stream))) {
                ret = HI_ERR_PNGD_ILLEGAL_PARAM;
            }
        }
    } else {
        ret = HI_ERR_PNGD_NULL_PTR;
    }

    return ret;
}

hi_s32 pngd_set_acl_param(hi_uintptr_t arg, hi_void *private_data)
{
    hi_s32 chn_id = UMAP_GET_CHN(private_data);
    hi_s32 ret = HI_FAILURE;
    ret = pngd_check_chn_valid(chn_id);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    return pngd_set_acl(chn_id, (pngd_acl_info *)arg);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif // end of #ifdef __cplusplus
