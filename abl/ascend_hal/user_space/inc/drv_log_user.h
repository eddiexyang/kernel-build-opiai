/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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

#ifndef DRV_LOG_USER_H
#define DRV_LOG_USER_H

#include <syslog.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

#include "ascend_hal.h"
#include "ascend_inpackage_hal.h"
#include "slog.h"

#ifdef __cplusplus
extern "C" {
#endif
const char *drv_log_get_module_str(enum devdrv_module_type module);
struct drv_log_print_info {
    uint32_t *conLogLevel;
    const char *(*logGetLevelString)(uint32_t level);
    const char *(*logGetPrintTime)(void);
    uint32_t (*loglevelShift)(uint32_t level);
    void (*logPrint)(int32_t moduleId, int32_t level, const char *fmt, ...);
};

int32_t errno_to_user_errno(int32_t kern_err_no);
drvError_t drvMngGetConsoleLogLevel(unsigned int *logLevel);
int32_t drv_log_out_handle_register(struct log_out_handle *handle, size_t input_size);
int32_t drv_log_out_handle_unregister(void);

#define TSDRV_SHARE_LOG_START (0xE0000000000ULL)
#define DEVMM_SHARE_LOG_START (0xE0000020000ULL)
#define DEVMNG_SHARE_LOG_START (0xE0000040000ULL)
#define HDC_SHARE_LOG_START   (0xE0000060000ULL)
#define SHARE_LOG_MAX_SIZE (4 * 1024)

void share_log_create(enum devdrv_module_type type, uint32_t size);
void share_log_destroy(enum devdrv_module_type type);
void share_log_read(enum devdrv_module_type type);

extern struct drv_log_print_info g_logPrintInfo;

#ifdef DRV_HOST
#define DRV_SYSLOG(LEVEL, module, fmt, ...) do { \
        if ((uint32_t)LEVEL <= *(g_logPrintInfo.conLogLevel)) { \
            g_logPrintInfo.logPrint(DRV, (int32_t)g_logPrintInfo.loglevelShift((uint32_t)LEVEL), \
                "[ascend]%s%s[curpid: %d, %d][drv][%s][%s %d]" fmt, \
                g_logPrintInfo.logGetLevelString((uint32_t)LEVEL), g_logPrintInfo.logGetPrintTime(), getpid(), \
                syscall(__NR_gettid), drv_log_get_module_str(module), \
                __func__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)
#define DRV_EMERG(module, fmt, ...) DRV_SYSLOG(LOG_EMERG, module, fmt, ##__VA_ARGS__)
#define DRV_ALERT(module, fmt, ...) DRV_SYSLOG(LOG_ALERT, module, fmt, ##__VA_ARGS__)
#define DRV_CRIT(module, fmt, ...) DRV_SYSLOG(LOG_CRIT, module, fmt, ##__VA_ARGS__)
#define DRV_ERR(module, fmt, ...) DRV_SYSLOG(LOG_ERR, module, fmt, ##__VA_ARGS__)
#define DRV_WARN(module, fmt, ...) DRV_SYSLOG(LOG_WARNING, module, fmt, ##__VA_ARGS__)
#define DRV_NOTICE(module, fmt, ...) DRV_SYSLOG(LOG_NOTICE, module, fmt, ##__VA_ARGS__)
#define DRV_INFO(module, fmt, ...) DRV_SYSLOG(LOG_INFO, module, fmt, ##__VA_ARGS__)
#define DRV_DEBUG(module, fmt, ...) DRV_SYSLOG(LOG_DEBUG, module, fmt, ##__VA_ARGS__)
#define DRV_EVENT(module, fmt, ...) DRV_CRIT(module, fmt, ##__VA_ARGS__)
#else
#define DRV_EMERG(module, fmt, ...) \
    dlog_error(DRV, "[%s] [%s %d] " fmt, drv_log_get_module_str(module), __func__, __LINE__, ##__VA_ARGS__)
#define DRV_ALERT(module, fmt, ...) \
    dlog_error(DRV, "[%s] [%s %d] " fmt, drv_log_get_module_str(module), __func__, __LINE__, ##__VA_ARGS__)
#define DRV_CRIT(module, fmt, ...) \
    dlog_event(DRV, "[%s] [%s %d] " fmt, drv_log_get_module_str(module), __func__, __LINE__, ##__VA_ARGS__)
#define DRV_ERR(module, fmt, ...) \
    dlog_error(DRV, "[%s] [%s %d] " fmt, drv_log_get_module_str(module), __func__, __LINE__, ##__VA_ARGS__)
#define DRV_WARN(module, fmt, ...) \
    dlog_warn(DRV, "[%s] [%s %d] " fmt, drv_log_get_module_str(module), __func__, __LINE__, ##__VA_ARGS__)
#define DRV_NOTICE(module, fmt, ...) \
    dlog_event(DRV, "[%s] [%s %d] " fmt, drv_log_get_module_str(module), __func__, __LINE__, ##__VA_ARGS__)
#define DRV_INFO(module, fmt, ...) \
    dlog_info(DRV, "[%s] [%s %d] " fmt, drv_log_get_module_str(module), __func__, __LINE__, ##__VA_ARGS__)
#define DRV_DEBUG(module, fmt, ...) \
    dlog_debug(DRV, "[%s] [%s %d] " fmt, drv_log_get_module_str(module), __func__, __LINE__, ##__VA_ARGS__)
#define DRV_EVENT(module, fmt, ...) DRV_CRIT(module, fmt, ##__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif
#endif /* _DRV_SYSLOG_USER_H_ */
