 /*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

#ifndef DVPP_EXPORTMACRO_H
#define DVPP_EXPORTMACRO_H

#if defined(__GNUC__) && ((__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))
#define DVPP_EXPORT __attribute__((visibility("default")))
#else
#define DVPP_EXPORT
#endif

#endif // DVPP_EXPORTMACRO_H