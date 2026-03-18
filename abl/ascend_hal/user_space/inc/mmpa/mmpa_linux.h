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

#ifndef MMPA_LINUX_MMPA_LINUX_H
#define MMPA_LINUX_MMPA_LINUX_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif  // __cpluscplus
#endif  // __cpluscplus

#define MMPA_MACINFO_DEFAULT_SIZE 18
#define MMPA_CPUDESC_DEFAULT_SIZE 64

typedef pthread_t mmThread;
typedef pthread_mutex_t mmMutex_t;
typedef pthread_cond_t mmCond;
typedef pthread_mutex_t mmMutexFC;
typedef pthread_rwlock_t mmRWLock_t;
typedef signed int mmProcess;
typedef int mmPollHandle;
typedef int mmPipeHandle;
typedef int mmFileHandle;
typedef int mmComPletionKey;
typedef int mmCompletionHandle;
typedef int mmErrorMsg;
typedef int mmFd_t;

typedef void *mmExitCode;
typedef key_t mmKey_t;
typedef int mmMsgid;
typedef struct dirent mmDirent;
typedef struct dirent mmDirent2;
typedef struct shmid_ds mmshmId_ds;
typedef int (*mmFilter)(const mmDirent *entry);
typedef int (*mmFilter2)(const mmDirent2 *entry);
typedef int (*mmSort)(const mmDirent **a, const mmDirent **b);
typedef int (*mmSort2)(const mmDirent2 **a, const mmDirent2 **b);
typedef size_t mmSize_t; //lint !e410 !e1051
typedef off_t mmOfft_t;
typedef pid_t mmPid_t;
typedef long MM_LONG;

typedef void *(*userProcFunc)(void *pulArg);

typedef struct {
  userProcFunc procFunc;  // Callback function pointer
  void *pulArg;           // Callback function parameters
} mmUserBlock_t;

typedef struct {
  const char *dli_fname;
  void *dli_fbase;
  const char *dli_sname;
  void *dli_saddr;
  size_t dli_size; /* ELF only */
  int dli_bind; /* ELF only */
  int dli_type;
} mmDlInfo;

typedef struct {
  int wSecond;             // Seconds. [0-60] (1 leap second)
  int wMinute;             // Minutes. [0-59]
  int wHour;               // Hours. [0-23]
  int wDay;                // Day. [1-31]
  int wMonth;              // Month. [1-12]
  int wYear;               // Year
  int wDayOfWeek;          // Day of week. [0-6]
  int tm_yday;             // Days in year.[0-365]
  int tm_isdst;            // DST. [-1/0/1]
  long int wMilliseconds;  // milliseconds
} mmSystemTime_t;

typedef sem_t mmSem_t;
typedef struct sockaddr mmSockAddr;
typedef socklen_t mmSocklen_t;
typedef int mmSockHandle;
typedef timer_t mmTimer;
typedef pthread_key_t mmThreadKey;

typedef int mmOverLap;

typedef ssize_t mmSsize_t;
typedef size_t mmSize; // size

typedef struct {
  unsigned int createFlag;
  signed int oaFlag;
} mmCreateFlag;

typedef struct {
  void *sendBuf;
  signed int sendLen;
} mmIovSegment;
typedef struct in_addr mmInAddr;

typedef struct {
  void *inbuf;
  signed int inbufLen;
  void *outbuf;
  signed int outbufLen;
  mmOverLap *oa;
} mmIoctlBuf;

typedef int mmAtomicType;
typedef int mmAtomicType64;

typedef enum {
  pollTypeRead = 1,  // pipe read
  pollTypeRecv,      // socket recv
  pollTypeIoctl,     // ioctl
} mmPollType;

typedef struct {
  mmPollHandle handle;            // The file descriptor or handle of poll is required
  mmPollType pollType;            // Operation type requiring poll
                                  // read or recv or ioctl
  signed int ioctlCode;                // IOCTL operation code, dedicated to IOCTL
  mmComPletionKey completionKey;  // The default value is blank, which is used in windows
                                  // The data used to receive the difference between which handle is readable
} mmPollfd;

typedef struct {
  void *priv;              // User defined private content
  mmPollHandle bufHandle;  // Value of handle corresponding to buf
  mmPollType bufType;      // Data types polled to
  void *buf;               // Data used in poll
  unsigned int bufLen;           // Data length used in poll
  unsigned int bufRes;           // Actual return length
} mmPollData, *pmmPollData;

typedef void (*mmPollBack)(pmmPollData);

typedef struct {
  signed int tz_minuteswest;  // How many minutes is it different from Greenwich
  signed int tz_dsttime;      // type of DST correction
} mmTimezone;

typedef struct {
  long long tv_sec;
  long long tv_usec;
} mmTimeval;

typedef struct {
  MM_LONG tv_sec;
  MM_LONG tv_nsec;
} mmTimespec;

typedef struct {
  unsigned long long totalSize;
  unsigned long long freeSize;
  unsigned long long availSize;
} mmDiskSize;

#define mmTLS __thread
typedef struct stat mmStat_t;
typedef struct stat64 mmStat64_t;
typedef mode_t mmMode_t;

typedef struct option mmStructOption;

typedef struct {
  char addr[MMPA_MACINFO_DEFAULT_SIZE];  // ex:aa-bb-cc-dd-ee-ff\0
} mmMacInfo;

typedef struct {
  char **argv;
  signed int argvCount;
  char **envp;
  signed int envpCount;
} mmArgvEnv;

typedef struct {
  char arch[MMPA_CPUDESC_DEFAULT_SIZE];
  char manufacturer[MMPA_CPUDESC_DEFAULT_SIZE];  // vendor
  char version[MMPA_CPUDESC_DEFAULT_SIZE];       // modelname
  signed int frequency;                               // cpu frequency
  signed int maxFrequency;                            // max speed
  signed int ncores;                                  // cpu cores
  signed int nthreads;                                // cpu thread count
  signed int ncounts;                                 // logical cpu nums
} mmCpuDesc;

typedef mode_t MODE;

typedef struct {
  signed int detachFlag;    // Determine whether to set separation property 0, not to separate 1
  signed int priorityFlag;  // Determine whether to set priority 0 and not set 1
  signed int priority;      // Priority value range to be set 1-99
  signed int policyFlag;    // Set scheduling policy or not 0 do not set 1 setting
  signed int policy;        // Scheduling policy value value
                       //  MMPA_THREAD_SCHED_RR
                       //  MMPA_THREAD_SCHED_OTHER
                       //  MMPA_THREAD_SCHED_FIFO
  signed int stackFlag;     // Set stack size or not: 0 does not set 1 setting
  unsigned int stackSize;    // The stack size unit bytes to be set cannot be less than MMPA_THREAD_STACK_MIN
} mmThreadAttr;

#ifdef __ANDROID__
#define S_IREAD S_IRUSR
#define S_IWRITE S_IWUSR
#endif

#define mm_no_argument        no_argument
#define mm_required_argument  required_argument
#define mm_optional_argument  optional_argument

#define M_FILE_RDONLY O_RDONLY
#define M_FILE_WRONLY O_WRONLY
#define M_FILE_RDWR O_RDWR
#define M_FILE_CREAT O_CREAT

#define M_RDONLY O_RDONLY
#define M_WRONLY O_WRONLY
#define M_RDWR O_RDWR
#define M_CREAT O_CREAT
#define M_BINARY O_RDONLY
#define M_TRUNC O_TRUNC
#define M_IRWXU S_IRWXU
#define M_APPEND O_APPEND

#define M_IN_CREATE IN_CREATE
#define M_IN_CLOSE_WRITE IN_CLOSE_WRITE
#define M_IN_IGNORED IN_IGNORED

#define M_OUT_CREATE IN_CREATE
#define M_OUT_CLOSE_WRITE IN_CLOSE_WRITE
#define M_OUT_IGNORED IN_IGNORED
#define M_OUT_ISDIR IN_ISDIR

#define M_IREAD S_IREAD
#define M_IRUSR S_IRUSR
#define M_IWRITE S_IWRITE
#define M_IWUSR S_IWUSR
#define M_IXUSR S_IXUSR
#define FDSIZE 64
#define M_MSG_CREAT IPC_CREAT
#define M_MSG_EXCL (IPC_CREAT | IPC_EXCL)
#define M_MSG_NOWAIT IPC_NOWAIT

#define M_WAIT_NOHANG WNOHANG  // Non blocking waiting
#define M_WAIT_UNTRACED \
  WUNTRACED  // If the subprocess enters the suspended state, it will return immediately
             // But the end state of the subprocess is ignored
#define M_UMASK_USRREAD S_IRUSR
#define M_UMASK_GRPREAD S_IRGRP
#define M_UMASK_OTHREAD S_IROTH

#define M_UMASK_USRWRITE S_IWUSR
#define M_UMASK_GRPWRITE S_IWGRP
#define M_UMASK_OTHWRITE S_IWOTH

#define M_UMASK_USREXEC S_IXUSR
#define M_UMASK_GRPEXEC S_IXGRP
#define M_UMASK_OTHEXEC S_IXOTH

#define mmConstructor(x) __attribute__((constructor)) void x()
#define mmDestructor(x) __attribute__((destructor)) void x()

#define MMPA_NO_ARGUMENT 0
#define MMPA_REQUIRED_ARGUMENT 1
#define MMPA_OPTIONAL_ARGUMENT 2

#define MMPA_MAX_PATH PATH_MAX
#define M_NAME_MAX MAX_FNAME

#define M_F_OK F_OK
#define M_X_OK X_OK
#define M_W_OK W_OK
#define M_R_OK R_OK


#define MM_DT_DIR DT_DIR
#define MM_DT_REG DT_REG

#define MMPA_STDIN STDIN_FILENO
#define MMPA_STDOUT STDOUT_FILENO
#define MMPA_STDERR STDERR_FILENO

#define MMPA_RTLD_NOW RTLD_NOW
#define MMPA_RTLD_GLOBAL RTLD_GLOBAL
#define MMPA_RTLD_LAZY RTLD_LAZY
#define MMPA_RTLD_NODELETE RTLD_NODELETE

#define MMPA_DL_EXT_NAME ".so"

#define MMPA_DLL_API

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif // __cpluscplus

#endif // MMPA_LINUX_MMPA_LINUX_H_
