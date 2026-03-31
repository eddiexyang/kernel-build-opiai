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

#ifndef HI_DVPP_PNGD_INTERNAL_H
#define HI_DVPP_PNGD_INTERNAL_H

#include "HiDvppCommon.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif // #ifdef __cplusplus

#define PNGD_MAX_CHN_NUM 128

/*********************************************************************************************/
/* Invalid channel ID. */
#define HI_ERR_PNGD_INVALID_CHN_ID 0xA0408002
/* At least one parameter is illegal ,eg, an illegal enumeration value. */
#define HI_ERR_PNGD_ILLEGAL_PARAM  0xA0408003
/* Channel exists. */
#define HI_ERR_PNGD_EXIST          0xA0408004
/* The channel does not exist. */
#define HI_ERR_PNGD_UNEXIST        0xA0408005
/* Using a NULL pointer. */
#define HI_ERR_PNGD_NULL_PTR       0xA0408006
/* Try to enable or initialize system, device or channel, before configure attribute. */
#define HI_ERR_PNGD_NOT_CFG        0xA0408007
/* Operation is not supported by NOW. */
#define HI_ERR_PNGD_NOT_SUPPORT    0xA0408008
/* Operation is not permitted, eg, try to change static attribute. */
#define HI_ERR_PNGD_NOT_PERM       0xA0408009
/* Failure caused by malloc memory. */
#define HI_ERR_PNGD_NO_MEM         0xA040800C
/* Failure caused by malloc buffer. */
#define HI_ERR_PNGD_NO_BUF         0xA040800D
/* No data in buffer. */
#define HI_ERR_PNGD_BUF_EMPTY      0xA040800E
/* No buffer for new data. */
#define HI_ERR_PNGD_BUF_FULL       0xA040800F
/* System is not ready, had not initialized or loaded. */
#define HI_ERR_PNGD_SYS_NOT_READY  0xA0408010
/* Bad address, eg. used for copy_from_user & copy_to_user. */
#define HI_ERR_PNGD_BAD_ADDR       0xA0408011
/* System busy */
#define HI_ERR_PNGD_BUSY           0xA0408012
/* hardware or software timeout */
#define HI_ERR_PNGD_TIMEOUT        0xA0408014
/* Internal system error. */
#define HI_ERR_PNGD_SYS_ERROR      0xA0408015


typedef HI_S32 PNGD_CHN;

typedef struct hiPIC_INFO_S {
    HI_VOID* pictureAddress;
    HI_U32 u32PicBufferSize;
    HI_U32 u32PicWidth;
    HI_U32 u32PicHeight;
    HI_U32 u32PicWidthStride;
    HI_U32 u32PicHeightStride;
    PIXEL_FORMAT_E pictureFormat;
} PIC_INFO_S;

typedef struct hiPNGD_CHN_ATTR_S {
    HI_U32 u32StreamQueCnt;  // reserved
    HI_U64 reserved[4];
} PNGD_CHN_ATTR_S;

typedef struct hiPNGD_CHN_ATTR_EX_S {
    PNGD_CHN_ATTR_S chanlAttr;
    HI_BOOL bEventNotify; // RW; Event Notify
    HI_VOID *userData;
} PNGD_CHN_ATTR_EX_S;

/*
 * @brief create png decoder channel
 * @param [in] pngdChn: png decoder channel id [0, PNGD_MAX_CHN_NUM)
 * @param [in] pstAttr: pointer of png decoder channel attribute
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_PNGD_CreateChn(PNGD_CHN pngdChn, const PNGD_CHN_ATTR_S *pstAttr);

/*
 * @brief destroy pngdChn channel
 * @param [in] pngdChn: png decoder channel id [0, PNGD_MAX_CHN_NUM)
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_PNGD_DestroyChn(PNGD_CHN pngdChn);

/*
 * @brief send stream and outbuffer to png decoder channel
 * @param [in] pngdChn: png decoder channel id [0, PNGD_MAX_CHN_NUM)
 * @param [in] pstStream: pointer of stream struct
 * @param [in] pstPngPicInfo: pointer of PIC_INFO_S struct
 * @param [in] s32MilliSec: -1 is block,0 is no block,other positive number is timeout
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_PNGD_SendStream(PNGD_CHN pngdChn, const IMG_STREAM_S *pstStream,
                              PIC_INFO_S *pstPngPicInfo, HI_S32 s32MilliSec);
/*
 * @brief get frame from png decoder channel
 * @param [in] pngdChn: png decoder channel id [0, PNGD_MAX_CHN_NUM)
 * @param [in] s32MilliSec: -1 is block,0 is no block,other positive number is timeout
 * @param [out] pstPngPicInfo: pointer of pic info struct
 * @param [out] pstStream: pointer of stream struct
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_PNGD_GetImageData(PNGD_CHN pngdChn, PIC_INFO_S *pstPngPicInfo,
                                IMG_STREAM_S *pstStream, HI_S32 s32MilliSec);

/*
 * @brief get input image's information parsed by dvpp
 * @param [in] pstPngStream: stream info pointer
 * @param [out] pstImgInfo: parsed image info pointer
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_PNG_GetImageInfo(const IMG_STREAM_S *pstPngStream, IMG_INFO_S *pstImgInfo);
/*
 * @brief create png decoder channel (For ACL)
 * @param [in] attr: pointer of png decoder channel attribute
 * @param [out] pngdChn: pointer of png decoder channel id [0, PNGD_MAX_CHN_NUM)
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_PNGD_CreateChnEx(PNGD_CHN *pngdChn, const PNGD_CHN_ATTR_EX_S *attr);

/*
 * @brief create png decoder channel (For  ACL-MPI)
 * @param [in] pngdChn: video decoder channel id [0, PNGD_MAX_CHN_NUM)
 * @param [in] attr: pointer of video decoder channel attribute
 * @param [in] flag: reserved for feautre use, must set 0 now
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_PNGD_CreateChnEx2(PNGD_CHN pngdChn, const PNGD_CHN_ATTR_S *attr, HI_U32 flag);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif // #ifdef __cplusplus

#endif // #ifndef HI_DVPP_PNGD_INTERNAL_H
