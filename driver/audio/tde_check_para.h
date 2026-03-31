/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: tde check
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */
#ifndef TDE_CHECK_PARA_H
#define TDE_CHECK_PARA_H

#include "ot_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define tde_check_array_over_return_value(cur_index, max_index, ret) \
    do {                                                           \
        if (cur_index >= max_index) {                                \
            return ret;                                            \
        }                                                          \
    } while (0)

#define tde_check_array_over_return_novalue(cur_index, max_index) \
    do {                                                        \
        if (cur_index >= max_index) {                             \
            return;                                             \
        }                                                       \
    } while (0)

#define tde_check_nullpointer_return_novalue(pointer) \
    do {                                              \
        if (pointer == TD_NULL) {                        \
            return;                                   \
        }                                             \
    } while (0)

#define tde_check_nullpointer_return_value(pointer, ret) \
    do {                                                 \
        if (pointer == TD_NULL) {                           \
            return ret;                                  \
        }                                                \
    } while (0)

#define tde_check_unequal_return_novalue(para1, para2) \
    do {                                               \
        if (para1 != para2) {                          \
            return;                                    \
        }                                              \
    } while (0)

#define tde_check_unequal_return_value(para1, para2, ret) \
    do {                                                  \
        if (para1 != para2) {                             \
            return ret;                                   \
        }                                                 \
    } while (0)

#define tde_check_equal_return_value(para1, para2, ret) \
    do {                                                \
        if (para1 == para2) {                           \
            return ret;                                 \
        }                                               \
    } while (0)

#define tde_check_equal_return_novalue(para1, para2) \
    do {                                             \
        if (para1 == para2) {                        \
            return;                                  \
        }                                            \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* TDE_CHECK_PARA_H */
