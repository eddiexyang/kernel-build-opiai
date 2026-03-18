/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
#ifdef __linux
#ifndef _MMPA_API_H_
#define _MMPA_API_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef FUNC_VISIBILITY
#define MMPA_FUNC_VISIBILITY __attribute__((visibility("default")))
#else
#define MMPA_FUNC_VISIBILITY
#endif

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <signal.h>
#include <pthread.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <net/if.h>
#include <limits.h>
#include <stddef.h>
#include <dirent.h>
#include <linux/fs.h>
#include <linux/limits.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/shm.h>
#include <sys/un.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/prctl.h>

#include "securec.h"

#include "mmpa_typedef_linux.h"
#include "mmpa_linux.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define MMPA_CPUINFO_DEFAULT_SIZE               64
#define MMPA_CPUINFO_DOUBLE_SIZE                128
#define MMPA_CPUPROC_BUF_SIZE                   256
#define MMPA_MAX_IF_SIZE                        2048
#define MMPA_SECOND_TO_MSEC                     1000
#define MMPA_MSEC_TO_USEC                       1000
#define MMPA_MSEC_TO_NSEC                       1000000
#define MMPA_SECOND_TO_NSEC                     1000000000
#define MMPA_MAX_SLEEP_MILLSECOND_USING_USLEEP  1000
#define MMPA_MAX_SLEEP_MICROSECOND_USING_USLEEP 1000000
#define MMPA_MAX_PHYSICALCPU_COUNT              4096
#define MMPA_MIN_PHYSICALCPU_COUNT              1
#define ERRNO_MIN                               (-255)


struct CpuTypeTable {
    char *key;
    char *value;
};

enum {
    MMPA_MAC_ADDR_FIRST_BYTE = 0,
    MMPA_MAC_ADDR_SECOND_BYTE,
    MMPA_MAC_ADDR_THIRD_BYTE,
    MMPA_MAC_ADDR_FOURTH_BYTE,
    MMPA_MAC_ADDR_FIFTH_BYTE,
    MMPA_MAC_ADDR_SIXTH_BYTE
};

/*
 * 描述:对设备的I/O通道进行管理
 * 参数: fd--指向设备驱动文件的文件资源描述符
 *       ioctlCode--ioctl的操作码
 *       bufPtr--指向数据的缓存, 里面包含的输入输出buf缓存由用户分配
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmIoctl(mmProcess fd, signed int ioctlCode, mmIoctlBuf *bufPtr)
{
    if ((fd < MMPA_ZERO) || bufPtr == NULL || bufPtr->inbuf == NULL) {
        return EN_INVALID_PARAM;
    }
    unsigned int request = (unsigned int)ioctlCode;
    signed int ret = ioctl(fd, request, bufPtr->inbuf);
    if (ret < ERRNO_MIN) {
        return EIO;
    } else if (ret < EN_OK) {
        return EN_ERROR;
    }

    return ret;
}
/*
 * 描述:判断文件或者目录是否存在
 * 参数: pathName -- 文件路径名
 * 参数: mode -- 权限
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmAccess2(const char *pathName, signed int mode)
{
    if (pathName == NULL) {
        return EN_INVALID_PARAM;
    }

    signed int ret = access(pathName, mode);
    if (ret != EN_OK) {
        return EN_ERROR;
    }
    return EN_OK;
}

/*
 * 描述:判断文件或者目录是否存在
 * 参数: pathName -- 文件路径名
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmAccess(const char *pathName)
{
    return mmAccess2(pathName, F_OK);
}

/*
 * 描述:把一个地址族中的特定地址赋给socket
 * 参数: sockFd--socket描述字，通过mmSocket函数创建
 *       addr--一个指向要绑定给sockFd的协议地址
 *       addrLen--对应地址的长度
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmBind(mmSockHandle sockFd, const mmSockAddr* addr, mmSocklen_t addrLen)
{
    if ((sockFd < MMPA_ZERO) || (addr == NULL) || (addrLen <= MMPA_ZERO)) {
        return EN_INVALID_PARAM;
    }

    signed int ret = bind(sockFd, addr, addrLen);
    if (ret != EN_OK) {
        return EN_ERROR;
    }
    return EN_OK;
}

/*
 * 描述:关闭打开的文件
 * 参数: fd--指向打开文件的资源描述符
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmClose(signed int fd)
{
    if (fd < MMPA_ZERO) {
        return EN_INVALID_PARAM;
    }

    signed int ret = close(fd);
    if (ret != EN_OK) {
        return EN_ERROR;
    }
    return EN_OK;
}

/*
 * 描述:关闭打开的文件或者设备驱动
 * 参数: 打开的文件描述符fileId
 * 返回值:执行成功返回EN_OK，执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmCloseFile(mmProcess fileId)
{
    if (fileId < MMPA_ZERO) {
        return EN_INVALID_PARAM;
    }

    signed int ret = close(fileId);
    if (ret != EN_OK) {
        return EN_ERROR;
    }
    return EN_OK;
}

/*
 * 描述:关闭建立的socket连接
 * 参数: sockFd--已打开或者建立连接的socket描述字
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmCloseSocket(mmSockHandle sockFd)
{
    if (sockFd < MMPA_ZERO) {
        return EN_INVALID_PARAM;
    }

    signed int ret = close(sockFd);
    if (ret != EN_OK) {
        return EN_ERROR;
    }
    return EN_OK;
}

/*
 * 描述:用默认的属性创建线程
 * 参数: threadHandle -- pthread_t类型的实例
 *       funcBlock --包含函数名和参数的结构体指针
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmCreateTask(mmThread *threadHandle, mmUserBlock_t *funcBlock)
{
    if ((threadHandle == NULL) || (funcBlock == NULL) || (funcBlock->procFunc == NULL)) {
        return EN_INVALID_PARAM;
    }

    signed int ret = pthread_create(threadHandle, NULL, funcBlock->procFunc, funcBlock->pulArg);
    if (ret != EN_OK) {
        ret = EN_ERROR;
    }
    return ret;
}

/*
 * 描述:创建带分离属性的线程, 不需要mmJoinTask等待线程结束
 * 参数: threadHandle -- pthread_t类型的实例
 *       funcBlock --包含函数名和参数的结构体指针
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmCreateTaskWithDetach(mmThread *threadHandle, mmUserBlock_t *funcBlock)
{
    if ((threadHandle == NULL) || (funcBlock == NULL) || (funcBlock->procFunc == NULL)) {
        return EN_INVALID_PARAM;
    }
    pthread_attr_t attr;
    (void)memset_s(&attr, sizeof(attr), 0, sizeof(attr)); /* unsafe_function_ignore: memset */

    // 初始化线程属性
    signed int ret = pthread_attr_init(&attr);
    if (ret != EN_OK) {
        return EN_ERROR;
    }
    // 设置默认线程分离属性
    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (ret != EN_OK) {
        (void)pthread_attr_destroy(&attr);
        return EN_ERROR;
    }

    ret = pthread_create(threadHandle, &attr, funcBlock->procFunc, funcBlock->pulArg);
    (void)pthread_attr_destroy(&attr);
    if (ret != EN_OK) {
        ret = EN_ERROR;
    }
    return ret;
}

/*
 * 描述:mmCreateTaskWithThreadAttr内部使用,设置线程调度相关属性
 * 参数: attr -- 线程属性结构体
 *       threadAttr -- 包含需要设置的线程属性类别和值
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline signed int LocalSetSchedAttr(pthread_attr_t *attr, const mmThreadAttr *threadAttr)
{
#ifndef __ANDROID__
            // 设置默认继承属性 PTHREAD_EXPLICIT_SCHED 使得调度属性生效
            if ((threadAttr->policyFlag == TRUE) || (threadAttr->priorityFlag == TRUE)) {
                if (pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED) != EN_OK) {
                    return EN_ERROR;
                }
            }
#endif

        // 设置调度策略
        if (threadAttr->policyFlag == TRUE) {
            if (threadAttr->policy != MMPA_THREAD_SCHED_FIFO && threadAttr->policy != MMPA_THREAD_SCHED_OTHER &&
                threadAttr->policy != MMPA_THREAD_SCHED_RR) {
                return EN_INVALID_PARAM;
            }
            if (pthread_attr_setschedpolicy(attr, threadAttr->policy) != EN_OK) {
                return EN_ERROR;
            }
        }

        // 设置优先级
        if (threadAttr->priorityFlag == TRUE) {
            if ((threadAttr->priority < MMPA_MIN_THREAD_PIO) || (threadAttr->priority > MMPA_MAX_THREAD_PIO)) {
                return EN_INVALID_PARAM;
            }
            struct sched_param param;//lint !e565
            (void)memset_s(&param, sizeof(param), 0, sizeof(param)); /* unsafe_function_ignore: memset */
            param.sched_priority = threadAttr->priority;
            if (pthread_attr_setschedparam(attr, &param) != EN_OK) {
                return EN_ERROR;
            }
        }
        return EN_OK;
}

/*
 * 描述:mmCreateTaskWithThreadAttr内部使用,创建带属性的线程, 支持调度策略、优先级、线程栈大小属性设置
 * 参数: attr -- 线程属性结构体
 *       threadAttr -- 包含需要设置的线程属性类别和值
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline signed int LocalSetThreadAttr(pthread_attr_t *attr, const mmThreadAttr *threadAttr)
{
    // 设置调度相关属性
    signed int ret = LocalSetSchedAttr(attr, threadAttr);
    if (ret != EN_OK) {
        return ret;
    }

    // 设置堆栈
    if (threadAttr->stackFlag == TRUE) {
        if (threadAttr->stackSize < MMPA_THREAD_MIN_STACK_SIZE) {
            return EN_INVALID_PARAM;
        }
        if (pthread_attr_setstacksize(attr, threadAttr->stackSize) != EN_OK) {
            return EN_ERROR;
        }
    }
    if (threadAttr->detachFlag == TRUE) {
        // 设置默认线程分离属性
        if (pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED) != EN_OK) {
            return EN_ERROR;
        }
    }
    return EN_OK;
}

/*
 * 描述:创建带属性的线程, 支持调度策略、优先级、线程栈大小、分离属性设置,除了分离属性,其他只在Linux下有效
 * 参数: threadHandle -- pthread_t类型的实例
 *       funcBlock --包含函数名和参数的结构体指针
 *       threadAttr -- 包含需要设置的线程属性类别和值
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmCreateTaskWithThreadAttr(mmThread *threadHandle, const mmUserBlock_t *funcBlock,
    const mmThreadAttr *threadAttr)
{
    if ((threadHandle == NULL) || (funcBlock == NULL) ||
        (funcBlock->procFunc == NULL) || (threadAttr == NULL)) {
        return EN_INVALID_PARAM;
    }

    pthread_attr_t attr;
    (void)memset_s(&attr, sizeof(attr), 0, sizeof(attr)); /* unsafe_function_ignore: memset */

    // 初始化线程属性
    signed int ret = pthread_attr_init(&attr);
    if (ret != EN_OK) {
        return EN_ERROR;
    }

    ret = LocalSetThreadAttr(&attr, threadAttr);
    if (ret != EN_OK) {
        (void)pthread_attr_destroy(&attr);
        return ret;
    }

    ret = pthread_create(threadHandle, &attr, funcBlock->procFunc, funcBlock->pulArg);
    (void)pthread_attr_destroy(&attr);
    if (ret != EN_OK) {
        ret = EN_ERROR;
    }
    return ret;
}

/*
 * 描述:获取错误码
 * 返回值:error code
 */
static inline MMPA_FUNC_VISIBILITY signed int mmGetErrorCode()
{
    signed int ret = (signed int)errno;
    return ret;
}

/*
 * 描述:获取系统开机到现在经过的时间
 * 返回值:执行成功返回类型mmTimespec结构的时间
 */
static inline MMPA_FUNC_VISIBILITY mmTimespec mmGetTickCount()
{
    mmTimespec rts = {0};
    struct timespec ts = {0};
    (void)clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    rts.tv_sec = ts.tv_sec;
    rts.tv_nsec = ts.tv_nsec;
    return rts;
}

/*
 * 描述:获取当前系统时间和时区信息, windows不支持时区获取
 * 参数:timeVal--当前系统时间, 不能为NULL
        timeZone--当前系统设置的时区信息, 可以为NULL, 表示不需要获取时区信息
 * 返回值:执行成功返回EN_OK, 失败返回EN_ERROR，入参错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmGetTimeOfDay(mmTimeval *timeVal, mmTimezone *timeZone)
{
    if (timeVal == NULL) {
        return EN_INVALID_PARAM;
    }
    signed int ret = gettimeofday((struct timeval *)timeVal, (struct timezone *)timeZone);
    if (ret != EN_OK) {
        ret = EN_ERROR;
    }
    return ret;
}

/*
 * 描述:监听socket
 * 参数: sockFd--socket描述字，通过mmSocket函数创建
 *       backLog--相应socket可以排队的最大连接个数
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmListen(mmSockHandle sockFd, signed int backLog)
{
    if ((sockFd < MMPA_ZERO) || (backLog <= MMPA_ZERO)) {
        return EN_INVALID_PARAM;
    }

    signed int ret = listen(sockFd, backLog);
    if (ret != EN_OK) {
        return EN_ERROR;
    }
    return EN_OK;
}

/*
 * 描述:把mutex锁删除
 * 参数: mutex--指向mmMutex_t 指针
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmMutexDestroy(mmMutex_t *mutex)
{
    if (mutex == NULL) {
        return EN_INVALID_PARAM;
    }

    signed int ret = pthread_mutex_destroy(mutex);
    if (ret != EN_OK) {
        ret = EN_ERROR;
    }
    return ret;
}

/*
 * 描述:用默认属性初始化锁
 * 参数: mutex --指向mmMutex_t 指针
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmMutexInit(mmMutex_t *mutex)
{
    if (mutex == NULL) {
        return EN_INVALID_PARAM;
    }

    signed int ret = pthread_mutex_init(mutex, NULL);
    if (ret != EN_OK) {
        ret = EN_ERROR;
    }

    return ret;
}

/*lint -e454*/
/*
 * 描述:把mutex锁住
 * 参数: mutex --指向mmMutex_t 指针
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmMutexLock(mmMutex_t *mutex)
{
    if (mutex == NULL) {
        return EN_INVALID_PARAM;
    }

    signed int ret = pthread_mutex_lock(mutex);
    if (ret != EN_OK) {
        ret = EN_ERROR;
    }
    return ret;
}
/*lint +e454*/

/*lint -e455*/
/*
 * 描述:把mutex锁解锁
 * 参数: mutex--指向mmMutex_t 指针
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmMutexUnLock(mmMutex_t *mutex)
{
    if (mutex == NULL) {
        return EN_INVALID_PARAM;
    }

    signed int ret = pthread_mutex_unlock(mutex);
    if (ret != EN_OK) {
        ret = EN_ERROR;
    }
    return ret;
}
/*lint +e455*/

/*
 * 描述:打开或者创建一个文件
 * 参数: pathName--需要打开或者创建的文件路径名，由用户确保绝对路径
 *       flags--打开或者创建的文件标志位
 *       mode -- 打开或者创建的权限
 * 返回值:执行成功返回对应打开的文件描述符, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmOpen2(const char *pathName, signed int flags, MODE mode)
{
    char *out_path = NULL;
    signed int fd = -1;
    if ((pathName == NULL) || (flags < MMPA_ZERO) ||
        (strnlen((pathName), (PATH_MAX + 1)) >= (PATH_MAX + 1))) {
        return EN_INVALID_PARAM;
    }
    unsigned int tmp = (unsigned int)flags;

    if (((tmp & (O_TRUNC | O_WRONLY | O_RDWR | O_CREAT)) == MMPA_ZERO) && (flags != O_RDONLY)) {
        return EN_INVALID_PARAM;
    }
    if (((mode & (S_IRUSR | S_IREAD)) == MMPA_ZERO) &&
        ((mode & (S_IWUSR | S_IWRITE)) == MMPA_ZERO)) {
        return EN_INVALID_PARAM;
    }
    if ((out_path = (char *)malloc(PATH_MAX + 1)) &&
        (!memset_s(out_path, (PATH_MAX + 1), 0, (PATH_MAX + 1))) &&
        (realpath((pathName), out_path))) {
        fd = open(out_path, flags, mode);
    }

    if (out_path != NULL) {
        free(out_path);
        out_path = NULL;
    }

    if (fd < MMPA_ZERO) {
        return EN_ERROR;
    }
    return fd;
}

/*
 * 描述:往打开的文件或者设备驱动读取内容
 * 参数: 打开的文件描述符fileId，buffer为用户分配缓存，len为buffer对应长度
 * 返回值:执行成功返回实际读取的长度，执行错误返回EN_ERROR，入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY mmSsize_t mmReadFile(mmProcess fileId, void *buffer, signed int len)
{
    if ((fileId < MMPA_ZERO) || (buffer == NULL) || (len < MMPA_ZERO)) {
        return EN_INVALID_PARAM;
    }
    unsigned int readLen = (unsigned int)len;

    mmSsize_t ret = read(fileId, buffer, readLen);
    if (ret < MMPA_ZERO) {
        return EN_ERROR;
    }
    return ret;
}

/*
 * 描述:设置当前执行的线程的线程名-线程体内调用, 仅在Linux下生效, Windows下不支持为空实现
 * 参数:name--需要设置的线程名
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmSetCurrentThreadName(const char* name)
{
    if (name == NULL) {
        return EN_INVALID_PARAM;
    }
    signed int ret = prctl(PR_SET_NAME, name);
    if (ret != EN_OK) {
        return EN_ERROR;
    }
    return EN_OK;
}

/*
 * 描述:睡眠指定时间
 * 参数: milliSecond -- 睡眠时间 单位ms, linux下usleep函数microSecond入参必须小于1000000
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmSleep(unsigned int milliSecond)
{
    if (milliSecond == MMPA_ZERO) {
        return EN_INVALID_PARAM;
    }
    unsigned int microSecond;

    // 防止截断
    if (milliSecond <= MMPA_MAX_SLEEP_MILLSECOND_USING_USLEEP) {
        microSecond = milliSecond * MMPA_MSEC_TO_USEC;
    } else {
        microSecond = MMPA_MAX_SLEEP_MICROSECOND_USING_USLEEP;
    }

    signed int ret = usleep(microSecond);
    if (ret != EN_OK) {
        return EN_ERROR;
    }
    return EN_OK;
}

/*
 * 描述:创建socket
 * 参数: sockFamily--协议域
 *       type--指定socket类型
 *       protocol--指定协议
 * 返回值:执行成功返回创建的socket id, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY mmSockHandle mmSocket(signed int sockFamily, signed int type, signed int protocol)
{
    signed int socketHandle = socket(sockFamily, type, protocol);
    if (socketHandle < MMPA_ZERO) {
        return EN_ERROR;
    }
    return socketHandle;
}

/*
 * 描述:往打开的文件或者设备驱动写内容
 * 参数:打开的文件描述符fileId，buffer为用户分配缓存，len为buffer对应长度
 * 返回值:执行成功返回EN_OK，执行错误返回EN_ERROR，入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY mmSsize_t mmWriteFile(mmProcess fileId, const void *buffer, signed int len)
{
    if ((fileId < MMPA_ZERO) || (buffer == NULL) || (len < MMPA_ZERO)) {
        return EN_INVALID_PARAM;
    }
    unsigned int writeLen = (unsigned int)len;
    mmSsize_t ret = write(fileId, buffer, writeLen);
    if (ret < MMPA_ZERO) {
        return EN_ERROR;
    }
    return ret;
}

/*
 * 描述:用于在一次函数调用中写多个非连续缓冲区
 * 参数: fd -- 打开的资源描述符
 *       iov -- 指向非连续缓存区的首地址的指针
 *       iovcnt -- 非连续缓冲区的个数, 最大支持MAX_IOVEC_SIZE片非连续缓冲区
 * 返回值:执行成功返回实际写入的长度, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY mmSsize_t mmWritev(mmSockHandle fd, mmIovSegment *iov, signed int iovcnt)
{
    signed int i = 0;
    if ((fd < MMPA_ZERO) || (iov == NULL) || (iovcnt < MMPA_ZERO) || (iovcnt > MAX_IOVEC_SIZE)) {
        return EN_INVALID_PARAM;
    }
    struct iovec tmpSegment[MAX_IOVEC_SIZE];
    (void)memset_s(tmpSegment, sizeof(tmpSegment), 0, sizeof(tmpSegment)); /* unsafe_function_ignore: memset */

    for (i = 0; i < iovcnt; i++) {
        tmpSegment[i].iov_base = iov[i].sendBuf;
        tmpSegment[i].iov_len = (unsigned int)iov[i].sendLen;
    }

    mmSsize_t ret = writev(fd, tmpSegment, iovcnt);
    if (ret < EN_OK) {
        return EN_ERROR;
    }
    return ret;
}

/*
 * 描述:创建一个消息队列用于进程间通信
 * 参数:key--消息队列的KEY键值,
 *      msgFlag--取消息队列标识符
 * 返回值:执行成功则返回打开的消息队列ID, 执行错误返回EN_ERROR
 */
static inline MMPA_FUNC_VISIBILITY mmMsgid mmMsgCreate(mmKey_t key, signed int msgFlag)
{
    return (mmMsgid)msgget(key, msgFlag);
}


/*
 * 描述:往消息队列发送消息
 * 参数:msqid--消息队列ID
 *      buf--由用户分配内存
 *      bufLen--缓存长度
 *      msgFlag--消息标志位
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmMsgSnd(mmMsgid msqid,
    const void *buf, signed int bufLen, signed int msgFlag)
{
    if ((buf == NULL) || (bufLen <= MMPA_ZERO)) {
        return EN_INVALID_PARAM;
    }
    unsigned int sndLen = (unsigned int)bufLen;
    return (signed int)msgsnd(msqid, buf, sndLen, msgFlag);
}

/*
 * 描述:从消息队列接收消息
 * 参数:msqid--消息队列ID
 *      bufLen--缓存长度
 *      msgFlag--消息标志位
 *      buf--由用户分配内存
 * 返回值:执行成功返回EN_OK, 执行错误返回EN_ERROR, 入参检查错误返回EN_INVALID_PARAM
 */
static inline MMPA_FUNC_VISIBILITY signed int mmMsgRcv(mmMsgid msqid,
    void *buf, signed int bufLen, signed int msgFlag)
{
    if ((buf == NULL) || (bufLen <= MMPA_ZERO)) {
        return EN_INVALID_PARAM;
    }
    unsigned int rcvLen = (unsigned int)bufLen;
    return (signed int)msgrcv(msqid, buf, rcvLen, MMPA_DEFAULT_MSG_TYPE, msgFlag);
}

static inline MMPA_FUNC_VISIBILITY signed int mmMsgCtl(mmMsgid msqid,
    int cmd, struct msqid_ds *buf)
{
    return (signed int)msgctl(msqid, cmd, buf);
}

#endif // MMPA_API_H_
#endif

