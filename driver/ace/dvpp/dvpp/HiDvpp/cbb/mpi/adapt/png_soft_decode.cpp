/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021. All rights reserved.
* Description:define pngd soft decode function
* Author: Hisilicon multimedia software group
* Create: 2021/07/22
*/

#include "png_soft_decode.h"

#include <unistd.h>
#include <cerrno> // errno
#include <string>
#include <sys/time.h> // gettimeodday
#include <unistd.h>
#include <unordered_map>
#include "hi_type.h"
#include "png.h"

using std::unordered_map;

namespace dvpp {
namespace Png {
constexpr uint32_t PNG_SIGNATURE_SIZE = 8; // Png格式图片签名,长度为8字节
// IHDR数据大小(4)(数值一般为13) + IHDR(4) + IHDR数据(13) + IHDR的CRC校验(4)
constexpr uint32_t PNG_IHDR_SIZE = 25;

// IHDR数据(13) 包括：
// 图像宽度(4) 以像素为单位
// 图像高度(4) 以像素为单位
// 图像深度(1) 索引彩色图像：1、2、4、8
//            灰度图像：1、2、4、8、16
//            真彩色图像：8、16
// 颜色类型(1) 0：灰度图像，1、2、4、8、16
//            2：真彩色图像，8、16
//            3：索引彩色图像，1、2、4、8
//            4：带a通道数据的灰度图像，8、16
//            4：带a通道数据的真彩色图像，8、16
// 压缩方法(1) LZ77派生算法
// 滤波器方法(1)
// 隔行扫描方法(4) 0：非隔行扫描
//                1：Adam7(由Adam M.Costello开发的7遍隔行扫描方法)
//
constexpr uint32_t PNG_RGB_TYPE_PIXEL_SIZE = 3; // RGB格式每个像素大小
constexpr uint32_t PNG_RGBA_TYPE_PIXEL_SIZE = 4; // RGBA格式每个像素大小

constexpr int32_t PNG_BIT_DEPTH_8BIT = 8;
constexpr uint32_t ALIGN_UP_SIZE_16  = 16;
constexpr uint32_t ALIGN_UP_SIZE_128 = 128;

constexpr uint32_t PNG_MAX_WIDTH = 4096; // 最大支持图片宽度
constexpr uint32_t PNG_MAX_HEIGHT = 4096; // 最大支持图片高度
constexpr uint32_t PNG_MIN_WIDTH = 32; // 最小支持图片宽度
constexpr uint32_t PNG_MIN_HEIGHT = 32; // 最小支持图片高度

constexpr uint32_t MAX_PNG_MEM_SIZE =
    PNG_MAX_WIDTH * PNG_MAX_HEIGHT * PNG_RGBA_TYPE_PIXEL_SIZE * 2; // 预留容量为长*宽*位大小的2倍

typedef struct {
    std::string name;
    hi_pixel_format pixel;
} PngPixelInfo;

const unordered_map<uint32_t, PngPixelInfo> g_libPngdOutFormat = {
    {PNG_COLOR_TYPE_RGB,        {"RGB",        PIXEL_FORMAT_RGB_888}   },
    {PNG_COLOR_TYPE_RGB_ALPHA,  {"RGB_Alpha",  PIXEL_FORMAT_RGBA_8888} },
    {PNG_COLOR_TYPE_GRAY,       {"Gray",       PIXEL_FORMAT_RGB_888}   },
    {PNG_COLOR_TYPE_GRAY_ALPHA, {"Gray_Alpha", PIXEL_FORMAT_RGBA_8888} } };

void PngShowPicInfo(png_structp pngPtr, png_infop infoPtr)
{
    HI_TRACE(HI_DBG_INFO, HI_ID_PNGD, " PNG. width:%u height:%u bit_depth:%u\n",
        png_get_image_width(pngPtr, infoPtr), png_get_image_height(pngPtr, infoPtr),
        png_get_bit_depth(pngPtr, infoPtr));
    HI_TRACE(HI_DBG_INFO, HI_ID_PNGD, " PNG. color_type:%u compression_type:%u\n",
        png_get_color_type(pngPtr, infoPtr), png_get_compression_type(pngPtr, infoPtr));
    HI_TRACE(HI_DBG_INFO, HI_ID_PNGD, " PNG. interlace_type:%u filter_type :%u\n",
        png_get_interlace_type(pngPtr, infoPtr), png_get_filter_type(pngPtr, infoPtr));
}

int32_t PngCheckValid(png_structp pngPtr, png_infop infoPtr, hi_pic_info *pngPicInfo)
{
    png_byte colorType = png_get_color_type(pngPtr, infoPtr);
    auto itor = g_libPngdOutFormat.find(colorType);
    if (itor == g_libPngdOutFormat.end()) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "Do not support png color type:(%u)!", colorType);
        return EN_ERR_ILLEGAL_PARAM;
    }
    if (pngPicInfo->picture_format == PIXEL_FORMAT_UNKNOWN) {
        pngPicInfo->picture_format = static_cast<PIXEL_FORMAT_E>(itor->second.pixel);
    }
    HI_TRACE(HI_DBG_INFO, HI_ID_PNGD, " Out pix_format:%u\n",
             pngPicInfo->picture_format);

    if ((pngPicInfo->picture_format == PIXEL_FORMAT_RGBA_8888) &&
        ((colorType == PNG_COLOR_TYPE_RGB) || (colorType == PNG_COLOR_TYPE_GRAY))) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " colorType:%s can't convert to RGBA_8888\n",
            itor->second.name.c_str());
        return EN_ERR_ILLEGAL_PARAM;
    }

    if ((pngPicInfo->picture_width < PNG_MIN_WIDTH) || (pngPicInfo->picture_width > PNG_MAX_WIDTH)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " width:%u invalid. it's out the range of [%u, %u]\n",
            pngPicInfo->picture_width, PNG_MIN_WIDTH, PNG_MAX_WIDTH);
        return EN_ERR_ILLEGAL_PARAM;
    }
    if ((pngPicInfo->picture_height < PNG_MIN_HEIGHT) || (pngPicInfo->picture_height > PNG_MAX_HEIGHT)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " height:%u invalid. it's out the range of [%u, %u]\n",
            pngPicInfo->picture_height, PNG_MIN_WIDTH, PNG_MAX_WIDTH);
        return EN_ERR_ILLEGAL_PARAM;
    }

    if ((pngPicInfo->picture_height_stride < pngPicInfo->picture_height) ||
        (pngPicInfo->picture_height_stride > ALIGN_UP(pngPicInfo->picture_height, ALIGN_UP_SIZE_128))) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " height_stride:%u invalid. it's out the range of [%u, %u]\n",
            pngPicInfo->picture_height_stride, pngPicInfo->picture_height,
            ALIGN_UP(pngPicInfo->picture_height, ALIGN_UP_SIZE_128));
        return EN_ERR_ILLEGAL_PARAM;
    }

    if (pngPicInfo->picture_buffer_size <
        (pngPicInfo->picture_width_stride * pngPicInfo->picture_height_stride)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, " picture_buf_size:%u is too small. it must bigger than %u\n",
            pngPicInfo->picture_buffer_size,
            (pngPicInfo->picture_width_stride * pngPicInfo->picture_height_stride));
        return EN_ERR_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

void PngSoftDecErrorExit(png_structp pngPtr, png_const_charp msg)
{
    HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "libpng error: %s\n", msg);
    throw std::runtime_error("run libpng get error");
}

void PngdUserReadData(png_structp pngPtr, png_bytep addr, size_t len)
{
    png_buf *softBuf = static_cast<png_buf *>(png_get_io_ptr(pngPtr));
    if (softBuf == nullptr) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "libpng copy softBuf ptr is nullptr!\n");
        return;
    }
    if (len > softBuf->left_size) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "libpng copy stream. read len:%u out board of left_len:%u\n",
            len, softBuf->left_size);
        return;
    }

    HI_TRACE(HI_DBG_DEBUG, HI_ID_PNGD, "libpng copy stream. read len:%u left len:%u\n", len, softBuf->left_size);

    int ret = memcpy_s(addr, len, softBuf->addr, len);
    if (ret != 0) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "libpng copy err. read_len:%u left_len:%u\n", len, softBuf->left_size);
    } else {
        softBuf->left_size -= len;
        softBuf->read_size += len;
        softBuf->addr += len;
    }
}


int32_t CallLibPng(png_structp pngPtr, png_infop infoPtr,
                   hi_pic_info *pngPicInfo, hi_img_stream *stream)
{
    try {
        // note: for error handle of third_party libpng, try...catch can used, according to c++ code standard
        png_buf softBuf{stream->addr, stream->len, 0, stream->len};
        // 替换libpng读函数. 否则libpng默认执行读文件操作会访问空fd
        png_set_read_fn(pngPtr, reinterpret_cast<void *>(&softBuf), PngdUserReadData);

        int32_t bitDepth;
        int32_t interlaceType;
        int32_t compressionType;
        int32_t filterType;
        int32_t colorType;
        png_read_info(pngPtr, infoPtr);
        png_get_IHDR(pngPtr, infoPtr, &pngPicInfo->picture_width, &pngPicInfo->picture_height,
            &bitDepth, &colorType, &interlaceType, &compressionType, &filterType);

        PngShowPicInfo(pngPtr, infoPtr);

        int32_t ret = PngCheckValid(pngPtr, infoPtr, pngPicInfo);
        if (ret != HI_SUCCESS) {
            return EN_ERR_ILLEGAL_PARAM;
        }

        if ((colorType == PNG_COLOR_TYPE_GRAY) || (colorType == PNG_COLOR_TYPE_GRAY_ALPHA)) {
            png_set_gray_to_rgb(pngPtr);
        }
        if ((colorType == PNG_COLOR_TYPE_GRAY_ALPHA) || (colorType == PNG_COLOR_TYPE_RGB_ALPHA)) {
            if (pngPicInfo->picture_format == PIXEL_FORMAT_RGB_888) {
                png_set_strip_alpha(pngPtr);
            }
        }
        if (bitDepth < PNG_BIT_DEPTH_8BIT) {
            png_set_expand(pngPtr);
        }

        png_read_update_info(pngPtr, infoPtr);

        png_bytep rowPointers[pngPicInfo->picture_height]{nullptr};
        for (uint32_t i = 0;  i < pngPicInfo->picture_height;  ++i) {
            rowPointers[i] = static_cast<png_bytep>(pngPicInfo->picture_address) +
                i * pngPicInfo->picture_width_stride;
        }
        png_read_image(pngPtr, rowPointers);

        png_read_end(pngPtr, infoPtr);
    } catch (...) {
        return EN_ERR_SYS_ERROR;
    }

    return HI_SUCCESS;
}

int32_t PngSoftDecode(hi_pic_info *pngPicInfo, hi_img_stream *stream)
{
    int ret = png_sig_cmp(reinterpret_cast<png_const_bytep>(stream->addr), 0, stream->len);
    if (ret != 0) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD,
            "input stream[addr:%#x, len:%u] is not a png file!", stream->addr, stream->len);
        return EN_ERR_ILLEGAL_PARAM;
    }

    png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
        nullptr, (png_error_ptr)PngSoftDecErrorExit, nullptr);
    if (!pngPtr) {
        return EN_ERR_SYS_ERROR;
    }
    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr) {
        png_destroy_read_struct(&pngPtr, nullptr, nullptr);
        return EN_ERR_SYS_ERROR;
    }

    ret = CallLibPng(pngPtr, infoPtr, pngPicInfo, stream);
    HI_TRACE(HI_DBG_INFO, HI_ID_PNGD, "png decode ret:%#x [stream addr:%p, len:%u]",
        ret, stream->addr, stream->len);

    png_destroy_info_struct(pngPtr, &infoPtr);
    png_destroy_read_struct(&pngPtr, nullptr, nullptr);

    return ret;
}

int32_t PngGetImageInfo(const uint8_t* data, uint32_t dataLen, IMG_INFO_S *imgInfo)
{
    uint8_t info8Bit[10] = {0}; // 存储输入的临时变量，长度为10字节，初始化为 0
    int32_t safeFuncRet = memcpy_s(info8Bit, sizeof(info8Bit),
                                   data + 16, sizeof(info8Bit)); // 偏移16 byte
    if (safeFuncRet != 0) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "memcpy_s failed ret = %d.", safeFuncRet);
        return EN_ERR_SYS_ERROR;
    }

    imgInfo->u32Width = (static_cast<uint32_t>(info8Bit[0]) << 24) | // 第0字节位移24位
                         (static_cast<uint32_t>(info8Bit[1]) << 16) | // 第1字节位移16位
                         (static_cast<uint32_t>(info8Bit[2]) << 8)  | // 第2字节位移8位
                          static_cast<uint32_t>(info8Bit[3]); // 组成32位整数，第3字节不用移位
    imgInfo->u32Height = (static_cast<uint32_t>(info8Bit[4]) << 24) | // 第4字节位移24位
                          (static_cast<uint32_t>(info8Bit[5]) << 16) | // 第5字节位移16位
                          (static_cast<uint32_t>(info8Bit[6]) << 8)  | // 第6字节位移8位
                           static_cast<uint32_t>(info8Bit[7]); // 组成32位整数，第7字节不用移位
    if ((imgInfo->u32Height < PNG_MIN_HEIGHT) || (imgInfo->u32Height > PNG_MAX_HEIGHT) ||
        (imgInfo->u32Width < PNG_MIN_WIDTH) || (imgInfo->u32Width > PNG_MAX_WIDTH)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "width:%u or height:%u invalid, it should between [%d, %d]",
            imgInfo->u32Width, imgInfo->u32Height, PNG_MIN_HEIGHT, PNG_MAX_HEIGHT);
        return EN_ERR_NOT_SUPPORT;
    }

    imgInfo->pngPixelFormat = static_cast<PNG_COLOR_FORMAT_E>(info8Bit[9]); // 第9个字节为颜色格式
    auto itor = g_libPngdOutFormat.find(imgInfo->pngPixelFormat);
    if (itor == g_libPngdOutFormat.end()) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "Do not support png color format:(%u)!", imgInfo->pngPixelFormat);
        imgInfo->u32ImgBufSize = 0;
        return EN_ERR_NOT_SUPPORT;
    }

    imgInfo->u32HeightStride = ALIGN_UP(imgInfo->u32Height, ALIGN_UP_SIZE_16);
    if (itor->second.pixel == PIXEL_FORMAT_RGB_888) {
        imgInfo->u32WidthStride = ALIGN_UP(imgInfo->u32Width, ALIGN_UP_SIZE_128) * PNG_RGB_TYPE_PIXEL_SIZE;
    } else if (itor->second.pixel == PIXEL_FORMAT_RGBA_8888) {
        imgInfo->u32WidthStride = ALIGN_UP(imgInfo->u32Width, ALIGN_UP_SIZE_128) * PNG_RGBA_TYPE_PIXEL_SIZE;
    }
    imgInfo->u32ImgBufSize = imgInfo->u32WidthStride * imgInfo->u32HeightStride;

    return HI_SUCCESS;
}
} // namespace Png
} // namespace Dvpp

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif // end of #ifdef __cplusplus

hi_s32 pngd_soft_decode(hi_pic_info *pngPicInfo, hi_img_stream *stream)
{
    return dvpp::Png::PngSoftDecode(pngPicInfo, stream);
}

hi_s32 pngd_soft_parse_stream(const hi_img_stream *stream, hi_img_info *stImgInfo)
{
    if ((stream->len < (dvpp::Png::PNG_SIGNATURE_SIZE + dvpp::Png::PNG_IHDR_SIZE)) ||
        (stream->len > dvpp::Png::MAX_PNG_MEM_SIZE)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD, "stream->len:%u out the range of [%u, %u]",
            stream->len, dvpp::Png::PNG_SIGNATURE_SIZE + dvpp::Png::PNG_IHDR_SIZE, dvpp::Png::MAX_PNG_MEM_SIZE);
        return EN_ERR_NOT_SUPPORT;
    }
    int ret = png_sig_cmp(stream->addr, 0, stream->len);
    if (ret != 0) {
        HI_TRACE(HI_DBG_ERR, HI_ID_PNGD,
            "input stream[addr:%#x, len:%u] is not a png file!", stream->addr, stream->len);
        return EN_ERR_NOT_SUPPORT;
    }

    return dvpp::Png::PngGetImageInfo(stream->addr, stream->len, stImgInfo);
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif // end of #ifdef __cplusplus
