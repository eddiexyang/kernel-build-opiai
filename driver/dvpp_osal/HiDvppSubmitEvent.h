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

#ifndef HI_DVPP_SUBMITEVENT_H
#define HI_DVPP_SUBMITEVENT_H

// DVPP sub event list
enum DvppSubEvent {
    VPC_COMPLETE_SUCCESS = 0x0,
    VPC_COMPLETE_FAIL = 0x1,
    VPC_FULL_TO_NOT_FULL = 0x2,

    JPEGD_COMPLETE_SUCCESS = 0x10,
    JPEGD_COMPLETE_FAIL = 0x11,

    JPEGE_COMPLETE_SUCCESS = 0x20,
    JPEGE_COMPLETE_FAIL = 0x21,
    JPEGE_SEND_TIMEOUT = 0x22,

    PNGD_COMPLETE_SUCCESS = 0x30,
    PNGD_COMPLETE_FAIL = 0x31,
    PNGD_FULL_TO_NOT_FULL = 0x32,
    PNGD_SEND_TIMEOUT = 0x33,

    VDEC_FULL_TO_NOT_FULL = 0x40,
    VDEC_COMPLETE = 0x41,
    VDEC_EOS = 0x42,
    VDEC_EMPTY_TO_NOT_EMPTY = 0x43,
    VDEC_SEND_TIMEOUT = 0x44,
    VDEC_CHN_ABNORMAL = 0X45,
    VDEC_EOS_ABNORMAL = 0X46,

    VENC_COMPLETE_SUCCESS = 0x50,
    VENC_COMPLETE_FAIL = 0x51,
    VENC_FULL_TO_NOT_FULL = 0x52,
    VENC_EMPTY_TO_NOT_EMPTY = 0x53,
    VENC_SEND_TIMEOUT = 0x54,

    DVPP_HDC_CONNECT = 0x80,
    DVPP_HDC_CLOSE = 0x81,
    DVPP_HDC_DATAIN = 0x82
};

struct JpegdAsyncOutMsg {
    uint32_t reserved;
};

struct JpegeAsyncOutMsg {
    uint32_t reserved;
};

struct VpcAsyncOutMsg {
    uint32_t ySum;
    uint32_t uSum;
    uint32_t vSum;
    uint32_t task_id;
    uint32_t is_histogram;
};

struct PngdAsyncOutMsg {
    uint32_t outDataSize;
    uint32_t imgWidth;
    uint32_t imgHeight;
    uint32_t imgWidthAligned; // Aligned width of the output image.
    uint32_t imgHeightAligned; // Aligned height of the output image.
    int32_t format; // 2表示RGB输出,6表示RGBA输出
};

struct VencAsyncOutMsg {
    uint32_t reserved;
};

struct VdecAsyncOutMsg {
    uint32_t channelId;
};

union DvppAsyncOutMsg {
    struct JpegdAsyncOutMsg jpegdOut;
    struct JpegeAsyncOutMsg jpegeOut;
    struct VpcAsyncOutMsg vpcOut;
    struct PngdAsyncOutMsg pngdOut;
    struct VencAsyncOutMsg vencOut;
    struct VdecAsyncOutMsg vdecOut;
};

// DVPP event message
struct DvppEventMsg {
    void* userData;
    union DvppAsyncOutMsg outMsg;
    int32_t retCode;
    int32_t chn_id;
};

struct HdcData {
    int32_t sessionFd;
    int32_t dataType;
    uint64_t srcAddr;
    uint32_t length;
};

// Dvpp Hdc Event Message
struct DvppHdcEventMsg {
    int32_t devId;
    int32_t vfid;
    int32_t peerPid;
    int32_t localPid;
    uint64_t deviceHdcStartTime;
    uint64_t deviceSubmitEventTime;
};

#endif // HI_DVPP_SUBMITEVENT_H

