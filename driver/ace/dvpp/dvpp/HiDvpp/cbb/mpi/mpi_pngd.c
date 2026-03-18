/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021. All rights reserved.
* Description:define interface of user
* Author: Hisilicon multimedia software group
* Create: 2021/07/22
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "hi_errno.h"
#include "pngd_drv.h"

#ifdef BUILD_MPI_ADAPTER
#include "hi_mpi_redefine.h"
#endif

#include "HiDvppPngdInternal.h"
#ifdef DVPP_PNGD_SUPPORT
#include "mpi_pngd_adapt.h"
#include "hi_comm_pngd_adapt.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif // #ifdef __cplusplus
#endif // #if __cplusplus

HI_S32 HI_MPI_PNGD_CreateChn(PNGD_CHN pngdChn, const PNGD_CHN_ATTR_S *pstAttr)
{
#ifdef DVPP_PNGD_SUPPORT
    MPI_ASSERT(sizeof(hi_pngd_chn_attr) == sizeof(PNGD_CHN_ATTR_S));

    return hi_mpi_pngd_create_chn(pngdChn, (hi_pngd_chn_attr *)pstAttr, HI_FALSE);
#else
    return HI_ERR_PNGD_NOT_SUPPORT; //lint !e569
#endif
}

HI_S32 HI_MPI_PNGD_CreateChnEx(PNGD_CHN *pngdChn, const PNGD_CHN_ATTR_EX_S *attr)
{
#ifdef DVPP_PNGD_SUPPORT
    MPI_ASSERT(sizeof(hi_pngd_chn_attr_ex) == sizeof(PNGD_CHN_ATTR_EX_S));

    return hi_mpi_pngd_create_chn_ex(pngdChn, (hi_pngd_chn_attr_ex *)attr);
#else
    return HI_ERR_PNGD_NOT_SUPPORT; //lint !e569
#endif
}

HI_S32 HI_MPI_PNGD_CreateChnEx2(PNGD_CHN pngdChn, const PNGD_CHN_ATTR_S *attr, HI_U32 flag)
{
#ifdef DVPP_PNGD_SUPPORT
    MPI_ASSERT(sizeof(hi_pngd_chn_attr) == sizeof(PNGD_CHN_ATTR_S));

    return hi_mpi_pngd_create_chn2(pngdChn, (const hi_pngd_chn_attr *)attr, flag);
#else
    return HI_ERR_PNGD_NOT_SUPPORT; //lint !e569
#endif
}

HI_S32 HI_MPI_PNGD_DestroyChn(PNGD_CHN pngdChn)
{
#ifdef DVPP_PNGD_SUPPORT
    return hi_mpi_pngd_destroy_chn(pngdChn);
#else
    return HI_ERR_PNGD_NOT_SUPPORT; //lint !e569
#endif
}

HI_S32 HI_MPI_PNGD_SendStream(PNGD_CHN pngdChn, const IMG_STREAM_S *pstStream,
                              PIC_INFO_S *pstPngPicInfo, HI_S32 s32MilliSec)
{
#ifdef DVPP_PNGD_SUPPORT
    MPI_ASSERT(sizeof(hi_img_stream) == sizeof(IMG_STREAM_S));
    MPI_ASSERT(sizeof(hi_pic_info) == sizeof(PIC_INFO_S));

    return hi_mpi_pngd_send_stream(pngdChn, (hi_img_stream *)pstStream,
                                   (hi_pic_info *)pstPngPicInfo, s32MilliSec);
#else
    return HI_ERR_PNGD_NOT_SUPPORT; //lint !e569
#endif
}

HI_S32 HI_MPI_PNGD_GetImageData(PNGD_CHN pngdChn, PIC_INFO_S *pstPngPicInfo,
                                IMG_STREAM_S *pstStream, HI_S32 s32MilliSec)
{
#ifdef DVPP_PNGD_SUPPORT
    MPI_ASSERT(sizeof(hi_img_stream) == sizeof(IMG_STREAM_S));
    MPI_ASSERT(sizeof(hi_pic_info) == sizeof(PIC_INFO_S));

    return hi_mpi_pngd_get_image_data(pngdChn, (hi_pic_info *)pstPngPicInfo,
                                      (hi_img_stream *)pstStream, s32MilliSec);
#else
    return HI_ERR_PNGD_NOT_SUPPORT; //lint !e569
#endif
}

HI_S32 HI_MPI_PNG_GetImageInfo(const IMG_STREAM_S *pstPngStream, IMG_INFO_S *pstImgInfo)
{
#ifdef DVPP_PNGD_SUPPORT
    MPI_ASSERT(sizeof(hi_img_stream) == sizeof(IMG_STREAM_S));
    MPI_ASSERT(sizeof(hi_img_info) == sizeof(IMG_INFO_S));

    return hi_mpi_png_get_image_info((hi_img_stream *)pstPngStream, (hi_img_info *)pstImgInfo);
#else
    return HI_ERR_PNGD_NOT_SUPPORT; //lint !e569
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif // #ifdef __cplusplus
#endif // #if __cplusplus

