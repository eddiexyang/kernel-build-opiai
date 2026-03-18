/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Description:
 * Author: huawei
 * Create: 2019-10-15
 */

#ifndef __DRV_FILE_UTIL_H__
#define __DRV_FILE_UTIL_H__
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static inline char *do_realpath(const char *filepath, char *regularization_path, int regular_len)
{
    char *path = NULL;

    if ((filepath == NULL) || (regularization_path == NULL)) {
        return NULL;
    }

    if (strnlen((filepath), (PATH_MAX + 1)) >= (PATH_MAX + 1)) {
        return NULL;
    }

    if (regular_len < PATH_MAX) {
        return NULL;
    }

    path = realpath(filepath, regularization_path);
    return path;
}

#endif
