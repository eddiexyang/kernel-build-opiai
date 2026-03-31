/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2019. All rights reserved.
* Description: Definitions of chip version: chipid, performance, functions, etc
* Author: Hisilicon multimedia software group
* Create: 2011/06/28
*/

#ifndef __HI_VERSION_H__
#define __HI_VERSION_H__

#include "hi_type.h"
#include "hi_board.h"
#include "hi_common_adapt.h"
#include "hi_comm_vb_adapt.h"

#include "vb_ext.h"
#include "hi_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define OTP_BASE                      0x204A0000  // 0x10250000 *2
#define TEST_ID                                       0x00U
#define COMM_ID_16CV500                               0x01U
#define COMM_EXT_ID_16CV500                           0x11U
#define HK_ID_16CV500                                 0x02U
#define HK_EXT_ID_16CV500                             0x12U
#define COMM_ID_16DV300                               0x05U
#define COMM_EXT_ID_16DV300                           0x15U
#define HK_ID_16DV300                                 0x06U
#define HK_EXT_ID_16DV300                             0x16U

#define COMM_ID_16AV300                               0x21U

#define COMM_ID_59V200                                0x09U
#define COMM_EXT_ID_59V200                            0x19U

#define COMM_ID_56V200                                0x0cU
#define COMM_EXT_ID_56V200                            0x1cU

#define COMM_ID_62V100                                0x41U

#define COMM_ID_66V100                                0x45U
#define INNER_SDK_VERSION_ID                          0x00U
#define INNER_SDK_VERSION_ADDR_OFFSET                 0x34U

#define OTP_CHIP_ID                   32
#define OTP_CPU_ADDR                  0
#define OTP_CPU_OP_START              0x0004
#define OTP_CPU_RW_FLAG               0x0008
#define OTP_CPU_RD_DATA               0x0010
#define OTP_CPU_OP_STA                0x0014

#define OTP_CUSTOMER_ID               768

extern hi_u32 venc_max_chn_num;

#define INNER_SDK_VERSION_ID                          0x00U
#define INNER_SDK_VERSION_ADDR_OFFSET                 0x34U
#define EFUSE_VENC_DEFAULT_SCENE_MODE() (0)

#define EFUSE_VENC_TOTAL_PERF_PROCESS(chn, W, H) (0)

#define EFUSE_VENC_TOTAL_OVER_PERF_PROCESS(chn, W, H) (0)

#define EFUSE_VENC_SINGLE_PERF_PROCESS(chn, W, H) (0)
#define INNER_SDK_VERSION_ID_IS_MATCH() ({       \
        hi_bool ret_ = HI_TRUE;        \
        ret_;                    \
    })

#define EFUSE_VENC_INIT_PERF_PROCESS()
#define EFUSE_VIU_FUNC_PROCESS()
#define EFUSE_VENC_FUNC_PROCESS()
#define VERIFY_INNER_SDK_VERSION_ID(ret)                      \
    do {                                                      \
        if (INNER_SDK_VERSION_ID_IS_MATCH()) { \
            (ret) = HI_SUCCESS;                                 \
        } else {                                              \
            (ret) = HI_FAILURE;                                 \
        }                                                     \
    } while (0)

#define EFUSE_VEDU_PERF_PROCESS()

#define EFUSE_GROUP_GET_TIME_INTERVAL(group, timeval) \
    do {                                                 \
        timeval = 0;                                     \
    } while (0)

#define OTP_GET_CHIPID(id)                                      \
    do {                                                        \
    } while (0)

#define CHIP_IS_TEST() ({       \
    hi_u32 chip = 0x123456;  \
    hi_bool ret_ = HI_FALSE;          \
    OTP_GET_CHIPID(chip);    \
    if (chip == TEST_ID)   { \
        ret_ = HI_TRUE;            \
    }                           \
    ret_;                    \
})
#define CHIP_IS_HI3516CV500() ({                                    \
        hi_bool ret_ = HI_TRUE;                                        \
        ret_;                                                           \
    })

#define CHIP_IS_HI3516DV300() ({                                    \
    hi_u32 chip = 0x123456;                                         \
    hi_bool ret_ = HI_FALSE;                                        \
    OTP_GET_CHIPID(chip);                                           \
    if ((chip == COMM_ID_16DV300) || (chip == HK_ID_16DV300)) {     \
        ret_ = HI_TRUE;                                             \
    }                                                               \
    ret_;                                                           \
})

#define CHIP_IS_HI3516AV300() ({        \
    hi_u32 chip = 0x123456;             \
    hi_bool ret_ = HI_FALSE;            \
    OTP_GET_CHIPID(chip);               \
    if (chip == COMM_ID_16AV300) {      \
        ret_ = HI_TRUE;                 \
    }                                   \
    ret_;                               \
})

#define CHIP_IS_HI3559V200() ({         \
    hi_u32 chip = 0x123456;             \
    hi_bool ret_ = HI_FALSE;            \
    OTP_GET_CHIPID(chip);               \
    if (chip == COMM_ID_59V200) {       \
        ret_ = HI_TRUE;                 \
    }                                   \
    ret_;                               \
})

#define CHIP_IS_HI3556V200() ({         \
    hi_u32 chip = 0x123456;             \
    hi_bool ret_ = HI_FALSE;            \
    OTP_GET_CHIPID(chip);               \
    if (chip == COMM_ID_56V200) {       \
        ret_ = HI_TRUE;                 \
    }                                   \
    ret_;                               \
})

#define chip_is_hi3562v100() ({         \
        hi_u32 chip = 0x123456;         \
        hi_bool ret_ = HI_FALSE;        \
        OTP_GET_CHIPID(chip);           \
        if (chip == COMM_ID_62V100) {   \
            ret_ = HI_TRUE;             \
        }                               \
        ret_;                           \
})

#define chip_is_hi3566v100() ({         \
        hi_u32 chip = 0x123456;         \
        hi_bool ret_ = HI_FALSE;        \
        OTP_GET_CHIPID(chip);           \
        if (chip == COMM_ID_66V100) {   \
            ret_ = HI_TRUE;             \
        }                               \
        ret_;                           \
})
/* 从VI获取数据处理后，用户送vpss到venc。
由于线程调度不均匀，导致送数据有快有慢，快的时候两帧间隔可能小于30ms。在18A的720P和960P模式下，
18C的720P模式下，客户是通过主动丢帧来规避的，前后丢个一两帧也问题不大，但这明显不是个好办法；
而在18C的960P的模式下，客户没办法规避。 */
#define EFUSE_VENC_GET_FRAME_RATE(venc_chn, frame_rate) \
    do {                                                    \
        frame_rate = 0xffffffff;                          \
    } while (0)

#define VERIFY_CHIPID(ret)                                                 \
    do {                                                                   \
        if (CHIP_IS_HI3516CV500()) {                                    \
            (ret) = HI_SUCCESS;                                              \
        } else {                                                           \
            (ret) = HI_FAILURE;                                              \
        }                                                                  \
    } while (0)

/* 芯片是否支持JPEGEDCF功能  */
#define CHIP_SUPPORT_JPEGEDCF()                                       \
    do {                                                              \
        if (ckfn_vb_is_supplement_support() == HI_FALSE) {            \
            return HI_FALSE;                                          \
        }                                                             \
        return call_vb_is_supplement_support(VB_SUPPLEMENT_JPEG_MASK) \
    } while (0)

/* 芯片是否支持DNG功能  */
#define CHIP_SUPPORT_DNG()                                           \
    do {                                                             \
        if (ckfn_vb_is_supplement_support() == HI_FALSE) {           \
            return HI_FALSE;                                         \
        }                                                            \
        return call_vb_is_supplement_support(VB_SUPPLEMENT_DNG_MASK) \
    } while (0)

#define CHIP_ROTATE_FUNC              0x1

extern hi_u32 g_func_id;


#define EFUSE_DVPP_VDEC_TOTAL_FRM    (22 * 31)
#define EFUSE_DVPP_VDEC_FRAME_WIDTH  1920
#define EFUSE_DVPP_VDEC_FRAME_HEIGHT 1080
#define EFUSE_DVPP_VDEC_FRAME_SIZE   (EFUSE_DVPP_VDEC_FRAME_HEIGHT * EFUSE_DVPP_VDEC_FRAME_WIDTH)
#define EFUSE_DVPP_VDEC_TOTAL_PER    (EFUSE_DVPP_VDEC_TOTAL_FRM * EFUSE_DVPP_VDEC_FRAME_SIZE)

#define IS_VDEC_PERFORMANCE_TOTAL_OVER(max_pixel_num) ({  \
    hi_bool ret_ = 0;                                   \
    hi_s32 idx = 0;                                     \
    hi_u32 total_key = 0;                                \
    for (idx = 0; idx < VdecMaxChnNum; idx++) {   \
        vdec_context *vdec_tmp_chn = &g_vdec_chn[idx]; \
        total_key += vdec_tmp_chn->total_perform;     \
    }                                                      \
    if (total_key >= max_pixel_num) {                   \
        ret_ = 1;                                       \
    }                                                      \
    if (total_key >= EFUSE_DVPP_VDEC_TOTAL_PER) {       \
        ret_ = 1;                                       \
    }                                                      \
    ret_;                                               \
})

#define EFUSE_VDEC_PERF_LIMIT(interval) ({                                                 \
    hi_bool ret_value = 0;                                                                      \
    hi_u64 max_pixel_num;                                                                    \
    max_pixel_num = osal_div64_u64(interval * EFUSE_DVPP_VDEC_TOTAL_PER, 1000000000ULL); \
    if ((IS_VDEC_PERFORMANCE_TOTAL_OVER(max_pixel_num)) || (interval > 1000000000ULL)) {  \
        ret_value = 1;                                                                          \
    }                                                                                         \
    ret_value;                                                                                  \
})

#define CHIP_SUPPORT_DEC() (1)


/* 芯片是否支持NNIE功能，hidvpp均支持，不做限制 */
/* 暂时CHIP ID 为0 也支持 */
#define CHIP_SUPPORT_NNIE() (1)
#define CHIP_SELECT_NNIE_MODE() (0x1)

/* 芯片是否支持IVE 车牌,FD功能 */
/* 暂时CHIP ID 为0 也支持 LPR FD  编译版本的时候改成#if 0 使用else分支下面的判断 */
#if 1
#define CHIP_SUPPORT_IVE_LPR_FD(ret_val, reg) \
    do {                                     \
        hi_u32 chip = 0x123456;           \
        EFUSE_GET_CHIP_ID(chip, reg);     \
        if (!((chip == 2))) {             \
            return ret_val;                   \
        }                                    \
    } while (0)
#else
#define CHIP_SUPPORT_IVE_LPR_FD(ret_val, reg)       \
    do {                                           \
        hi_u32 chip = 0x123456;                 \
        EFUSE_GET_CHIP_ID(chip, reg);           \
        if (!((chip == 2) || (0 == chip))) { \
            return ret_val;                         \
        }                                          \
    } while (0)

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
