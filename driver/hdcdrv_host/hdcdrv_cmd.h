/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#ifndef _HDCDRV_CMD_H_
#define _HDCDRV_CMD_H_

#define HDCDRV_CHAR_DRIVER_NAME "hisi_hdc"

#define HDCDRV_SERVICE_TYPE_DMP 0
#define HDCDRV_SERVICE_TYPE_PROFILING 1 // use for profiling tool
#define HDCDRV_SERVICE_TYPE_IDE1 2
#define HDCDRV_SERVICE_TYPE_FILE_TRANS 3
#define HDCDRV_SERVICE_TYPE_IDE2 4
#define HDCDRV_SERVICE_TYPE_LOG 5
#define HDCDRV_SERVICE_TYPE_RDMA 6
#define HDCDRV_SERVICE_TYPE_BBOX 7
/* the follow used in cloud */
#define HDCDRV_SERVICE_TYPE_FRAMEWORK 8
#define HDCDRV_SERVICE_TYPE_TSD 9
#define HDCDRV_SERVICE_TYPE_TDT 10
#define HDCDRV_SERVICE_TYPE_PROF 11 // use for drv prof
#define HDCDRV_SERVICE_TYPE_IDE_FILE_TRANS 12
#define HDCDRV_SERVICE_TYPE_DUMP 13
#define HDCDRV_SERVICE_TYPE_USER3 14 // use for test
#define HDCDRV_SERVICE_TYPE_DVPP 15
#define HDCDRV_SERVICE_TYPE_QUEUE 16
#define HDCDRV_SERVICE_TYPE_UPGRADE 17
#define HDCDRV_SERVICE_TYPE_APPLY_MAX 18 // update this value if add new service not for user
#define HDCDRV_SERVICE_TYPE_USER_START 64
#define HDCDRV_SERVICE_TYPE_USER_END 127
#define HDCDRV_SUPPORT_MAX_SERVICE 128

#define HDCDRV_SUPPORT_MAX_FID_PID  HDCDRV_SUPPORT_MAX_SERVICE

#define HDCDRV_DEV_ID_DEFAULT ((int)(-1))

#define HDCDRV_DEV_MAX_VDEV_PER_DEVICE 16

extern unsigned int hdcdrv_dev_num;

#ifdef CFG_FEATURE_SRIOV
#ifdef HDC_ENV_DEVICE
#define HDCDRV_SUPPORT_MAX_DEV 64
#else
#define HDCDRV_SUPPORT_MAX_DEV 1124
#endif
#else
#ifdef CFG_FEATURE_HDC_REG_MEM
#define HDCDRV_SUPPORT_MAX_DEV 2
#else
#ifdef HDC_ENV_DEVICE
#define HDCDRV_SUPPORT_MAX_DEV 4
#else
#define HDCDRV_SUPPORT_MAX_DEV 64
#endif
#endif
#endif

#if defined(CFG_FEATURE_VFIO_DEVICE) || defined(CFG_FEATURE_VFIO)
#define HDCDRV_SINGLE_DEV_MAX_SESSION       ((136 * HDCDRV_DEV_MAX_VDEV_PER_DEVICE) + 8)
#define HDCDRV_SINGLE_DEV_MAX_SHORT_SESSION ((8 * HDCDRV_DEV_MAX_VDEV_PER_DEVICE) + 8)
#else
#if defined(CFG_FEATURE_SRIOV) && !defined(HDC_ENV_DEVICE)
#define HDCDRV_SINGLE_DEV_MAX_SESSION       ((136 * HDCDRV_DEV_MAX_VDEV_PER_DEVICE) + 8)
#define HDCDRV_SINGLE_DEV_MAX_SHORT_SESSION ((8 * HDCDRV_DEV_MAX_VDEV_PER_DEVICE) + 8)
#else
#define HDCDRV_SINGLE_DEV_MAX_SHORT_SESSION 8
#if (defined(CFG_FEATURE_SRIOV) && defined(HDC_ENV_DEVICE)) || defined(DCFG_FEATURE_PROCESS_NUM_LITE)
#define HDCDRV_SINGLE_DEV_MAX_SESSION       136 /* 32 * 4(log + tsd + dvpp + reserved) + 8 */
#else
#define HDCDRV_SINGLE_DEV_MAX_SESSION       264 /* 64 * 4(log + tsd + dvpp + reserved) + 8 */
#endif
#endif
#endif
#define HDCDRV_SINGLE_DEV_MAX_LONG_SESSION  (HDCDRV_SINGLE_DEV_MAX_SESSION - HDCDRV_SINGLE_DEV_MAX_SHORT_SESSION)

#define HDCDRV_SUPPORT_MAX_SESSION          (HDCDRV_SINGLE_DEV_MAX_SESSION * HDCDRV_SUPPORT_MAX_DEV)
#define HDCDRV_SUPPORT_MAX_SHORT_SESSION    (HDCDRV_SINGLE_DEV_MAX_SHORT_SESSION * HDCDRV_SUPPORT_MAX_DEV)
#define HDCDRV_SUPPORT_MAX_LONG_SESSION     (HDCDRV_SUPPORT_MAX_SESSION - HDCDRV_SUPPORT_MAX_SHORT_SESSION)

#define HDCDRV_REAL_MAX_SESSION          ((int)(HDCDRV_SINGLE_DEV_MAX_SESSION * (int)hdcdrv_dev_num))
#define HDCDRV_REAL_MAX_SHORT_SESSION    (HDCDRV_SINGLE_DEV_MAX_SHORT_SESSION * (int)hdcdrv_dev_num)
#define HDCDRV_REAL_MAX_LONG_SESSION     (HDCDRV_REAL_MAX_SESSION - HDCDRV_REAL_MAX_SHORT_SESSION)

static inline int hdcdrv_get_max_support_dev(void)
{
#ifdef HDC_ENV_DEVICE
#ifdef CFG_FEATURE_SRIOV
    return (int)HDCDRV_SUPPORT_MAX_DEV;
#else
    return (int)hdcdrv_dev_num;
#endif
#else
    return (int)HDCDRV_SUPPORT_MAX_DEV;
#endif
}

#define HDCDRV_SUPPORT_MAX_DEV_MSG_CHAN 64

#define HDCDRV_SRIOV_VF_SUPPORT_MAX_NORMAL_MSG_CHAN 2
#ifdef CFG_FEATURE_VFIO
#define HDCDRV_SUPPORT_MAX_DEV_NORMAL_MSG_CHAN (HDCDRV_DEV_MAX_VDEV_PER_DEVICE * 2)
#else
#define HDCDRV_SUPPORT_MAX_DEV_NORMAL_MSG_CHAN HDCDRV_SUPPORT_MAX_SERVICE
#endif

#define HDCDRV_SID_LEN 32

#define HDCDRV_OK 0
#define HDCDRV_ERR (-1)
#define HDCDRV_PARA_ERR (-2)
#define HDCDRV_COPY_FROM_USER_FAIL (-3)
#define HDCDRV_COPY_TO_USER_FAIL (-4)
#define HDCDRV_SERVICE_LISTENING (-5)
#define HDCDRV_SERVICE_NO_LISTENING (-6)
#define HDCDRV_SERVICE_ACCEPTING (-7)
#define HDCDRV_DMA_MEM_ALLOC_FAIL (-8)
#define HDCDRV_NO_SESSION (-9)
#define HDCDRV_SEND_CTRL_MSG_FAIL (-10)
#define HDCDRV_REMOTE_REFUSED_CONNECT (-11)
#define HDCDRV_CONNECT_TIMEOUT (-12)
#define HDCDRV_TX_QUE_FULL (-13)
#define HDCDRV_TX_LEN_ERR (-14)
#define HDCDRV_TX_REMOTE_CLOSE (-15)
#define HDCDRV_RX_BUF_SMALL (-16)
#define HDCDRV_DEVICE_NOT_READY (-17)
#define HDCDRV_DEVICE_RESET (-18)
#define HDCDRV_NOT_SUPPORT (-19)
#define HDCDRV_REMOTE_SERVICE_NO_LISTENING (-20)
#define HDCDRV_NO_BLOCK (-21)
#define HDCDRV_SESSION_HAS_CLOSED (-22)
#define HDCDRV_MEM_NOT_MATCH (-23)
#define HDCDRV_CONV_FAILED (-24)
#define HDC_LOW_POWER_STATE (-25)
#define HDCDRV_NO_EPOLL_FD (-26)
#define HDCDRV_RX_TIMEOUT (-27)
#define HDCDRV_TX_TIMEOUT (-28)
#define HDCDRV_DMA_MEM_ISUSED (-29)
#define HDCDRV_SESSION_ID_MISS_MATCH (-30)
#define HDCDRV_MEM_ALLOC_FAIL (-31)
#define HDCDRV_SQ_DESC_NULL (-32)
#define HDCDRV_F_NODE_SEARCH_FAIL (-33)
#define HDCDRV_DMA_COPY_FAIL (-34)
#define HDCDRV_SAFE_MEM_OP_FAIL (-35)
#define HDCDRV_CHAR_DEV_CREAT_FAIL (-36)
#define HDCDRV_DMA_MPA_FAIL (-37)
#define HDCDRV_FIND_VMA_FAIL (-38)
#define HDCDRV_DMA_QUE_FULL (-39)
#define HDCDRV_CMD_CONTINUE (-40)
#define HDCDRV_NO_PERMISSION (-41)
#define HDCDRV_EPOLL_CLOSE (-42)
#define HDCDRV_SESSION_CHAN_INVALID (-43)
#define HDCDRV_GET_NUMA_ID_FAILED (-44)
#define HDCDRV_NO_WAIT_MEM_INFO (-45)
#define HDCDRV_NO_WAIT_MEM_TIMEOUT (-46)


/* if add a new error code need to add the same str in g_errno_str */
#define HDCDRV_WAIT_ALWAYS 0
#define HDCDRV_NOWAIT 1
#define HDCDRV_WAIT_TIMEOUT 2

#define HDCDRV_CMD_MAGIC 'H'

#define HDCDRV_MODE_DEFAULT 0
#define HDCDRV_MODE_CONTAINER 1


#define HDCDRV_SESSION_RUN_ENV_UNKNOW 0
#define HDCDRV_SESSION_RUN_ENV_PHYSICAL 1
#define HDCDRV_SESSION_RUN_ENV_PHYSICAL_CONTAINER 2
#define HDCDRV_SESSION_RUN_ENV_VIRTUAL 3
#define HDCDRV_SESSION_RUN_ENV_VIRTUAL_CONTAINER 4

#define HDCDRV_MEM_MAX_LEN (512 * 1024 * 1024)
#define HDCDRV_CTRL_MEM_MAX_LEN (256 * 1024)
#define HDCDRV_MEM_MIN_LEN (4 * 1024)
#define HDCDRV_MEM_CACHE_LIMIT (64 * 1024)
#define HDCDRV_MEM_MAX_NUM (HDCDRV_MEM_MAX_LEN / HDCDRV_MEM_MIN_LEN)
#define HDCDRV_MEM_MAX_LEN_BIT 22      /* 4M */
#define HDCDRV_MEM_1MB_LEN_BIT 20      /* 1M */
#define HDCDRV_MEM_512KB_LEN_BIT 19    /* 512k */
#define HDCDRV_MEM_MIN_LEN_BIT 18      /* 256k */
#define HDCDRV_MEM_64KB_LEN_BIT 16      /* 64kb */

#define HDCDRV_MEM_ORDER_1MB 8      /* 1M */


#define HDCDRV_MEM_MIN_PAGE_LEN_BIT PAGE_SHIFT
#define HDCDRV_MEM_MIN_HUGE_PAGE_LEN_BIT HPAGE_SHIFT

#define HDCDRV_MEM_ORDER_NUM 11 /* 4M order is 10 */
#define HDCDRV_MEM_SCORE_SCALE 100

#define HDCDRV_FAST_MEM_TYPE_TX_DATA 0
#define HDCDRV_FAST_MEM_TYPE_TX_CTRL 1
#define HDCDRV_FAST_MEM_TYPE_RX_DATA 2
#define HDCDRV_FAST_MEM_TYPE_RX_CTRL 3
#define HDCDRV_FAST_MEM_TYPE_DVPP 4
#define HDCDRV_FAST_MEM_TYPE_ANY 5
#define HDCDRV_FAST_MEM_TYPE_MAX  6
#define HDCDRV_PAGE_TYPE_NORMAL 0
#define HDCDRV_PAGE_TYPE_HUGE 1
#define HDCDRV_PAGE_TYPE_NONE 2
#define HDCDRV_PAGE_TYPE_REGISTER 3

#define HDCDRV_DEFAULT_DEV_ID 0
#define HDCDRV_DEFAULT_PM_FID 0
#define HDCDRV_DEFAULT_LOCAL_FID 0
#define HDCDRV_INVALID_FID (unsigned int)(-1)

#define HDCDRV_INVALID_PEER_PID (-1)

#define HDCDRV_SESSION_OWNER_PM 0
#define HDCDRV_SESSION_OWNER_VM 1
#define HDCDRV_SESSION_OWNER_CT 2

#define HDCDRV_DEFAULT_VM_ID 0 /* physical */

#if defined(CFG_FEATURE_VFIO_DEVICE) || defined(CFG_FEATURE_VFIO)
#define HDCDRV_VM_NUM (HDCDRV_DEV_MAX_VDEV_PER_DEVICE * hdcdrv_dev_num)
#define HDCDRV_EPOLL_FD_NUM (HDCDRV_VM_NUM * 128)
#else
#if defined(CFG_FEATURE_SRIOV) && !defined(HDC_ENV_DEVICE)
#define HDCDRV_VM_NUM (HDCDRV_DEV_MAX_VDEV_PER_DEVICE * hdcdrv_dev_num)
#define HDCDRV_EPOLL_FD_NUM (HDCDRV_VM_NUM * 128)
#else
#define HDCDRV_VM_NUM hdcdrv_dev_num
#define HDCDRV_EPOLL_FD_NUM (128 * hdcdrv_dev_num)
#endif
#endif
#define HDCDRV_MAX_VM_NUM (HDCDRV_VM_NUM + 1)

#define HDCDRV_EPOLL_FD_EVENT_NUM 1024
#define HDCDRV_VEPOLL_EVENT_MAX 5
#define HDCDRV_SESSION_RX_LIST_MAX_PKT 8

#define HDCDRV_RETRY_SLEEP_TIME 5 /* ms */

enum hdcdrv_cmd_type {
    HDCDRV_CMD_GET_PEER_DEV_ID = 0x7,
    HDCDRV_CMD_CONFIG = 0x8,
    HDCDRV_CMD_SET_SERVICE_LEVEL = 0x9,
    HDCDRV_CMD_SERVER_CREATE = 0x10,
    HDCDRV_CMD_SERVER_DESTROY = 0x11,
    HDCDRV_CMD_ACCEPT = 0x12,
    HDCDRV_CMD_CONNECT = 0x13,
    HDCDRV_CMD_CLOSE = 0x14,
    HDCDRV_CMD_SEND = 0x15,
    HDCDRV_CMD_RECV_PEEK = 0x16,
    HDCDRV_CMD_RECV = 0x17,
    HDCDRV_CMD_SET_SESSION_OWNER = 0x18,
    HDCDRV_CMD_GET_STAT = 0x19,
    HDCDRV_CMD_GET_SESSION_ATTR = 0x1a,
    HDCDRV_CMD_SET_SESSION_TIMEOUT = 0x1b,
    HDCDRV_CMD_GET_SESSION_UID = 0x1c,
    HDCDRV_CMD_GET_PAGE_SIZE = 0x1d,
    HDCDRV_CMD_GET_SESSION_INFO = 0x1e,
    HDCDRV_CMD_ALLOC_MEM = 0x20,
    HDCDRV_CMD_FREE_MEM = 0x21,
    HDCDRV_CMD_FAST_SEND = 0x22,
    HDCDRV_CMD_FAST_RECV = 0x23,
    HDCDRV_CMD_DMA_MAP = 0x24,
    HDCDRV_CMD_DMA_UNMAP = 0x25,
    HDCDRV_CMD_DMA_REMAP = 0x26,
    HDCDRV_CMD_REGISTER_MEM = 0x27,
    HDCDRV_CMD_UNREGISTER_MEM = 0x28,
    HDCDRV_CMD_WAIT_MEM = 0x29,
    HDCDRV_CMD_EPOLL_ALLOC_FD = 0x40,
    HDCDRV_CMD_EPOLL_FREE_FD = 0x41,
    HDCDRV_CMD_EPOLL_CTL = 0x42,
    HDCDRV_CMD_EPOLL_WAIT = 0x43,
    HDCDRV_CMD_MAX
};

enum hdcdrv_session_attr_cmd_type {
    HDCDRV_SESSION_ATTR_RUN_ENV = 0,
    HDCDRV_SESSION_ATTR_VFID,
    HDCDRV_SESSION_ATTR_LOCAL_CREATE_PID,
    HDCDRV_SESSION_ATTR_PEER_CREATE_PID,
    HDCDRV_SESSION_ATTR_STATUS,
    HDCDRV_SESSION_ATTR_DFX,
    HDCDRV_SESSION_ATTR_MAX
};

struct hdcdrv_timeout {
    unsigned int send_timeout;
    unsigned int recv_timeout;
    unsigned int fast_send_timeout;
    unsigned int fast_recv_timeout;
};

struct hdcdrv_event {
    unsigned int events;
    int sub_data;       /* trans to user in epoll wait return */
    unsigned long long data; /* trans to user in epoll wait return */
};

/* same as HDC_EPOLL_OP_* */
#define HDCDRV_EPOLL_OP_ADD 0
#define HDCDRV_EPOLL_OP_DEL 1

/* same as HDC_EPOLL_* */
#define HDCDRV_EPOLL_CONN_IN (0x1 << 0)
#define HDCDRV_EPOLL_DATA_IN (0x1 << 1)
#define HDCDRV_EPOLL_FAST_DATA_IN (0x1 << 2)
#define HDCDRV_EPOLL_SESSION_CLOSE (0x1 << 3)

#define HDCDRV_EPOLL_CTL_PARA_NUM    4

#define HDCDRV_RUNNING_NORMAL 0
#define HDCDRV_RUNNING_SUSPEND 1
#define HDCDRV_RUNNING_RESUME 2 // rcv resume callback

struct hdcdrv_mem_stat {
    int mem_nums[HDCDRV_FAST_MEM_TYPE_MAX];
    unsigned long long mem_size[HDCDRV_FAST_MEM_TYPE_MAX];
};

struct hdcdrv_cmd_common {
    int ret;
    int dev_id;
    unsigned long long pid;
};

struct hdcdrv_cmd_get_peer_dev_id {
    int ret;
    int dev_id;         /* input */
    unsigned long long pid;
    int peer_dev_id;    /* output */
};

struct hdcdrv_cmd_config {
    int ret;
    int dev_id;
    unsigned long long pid;     /* input */
    int segment; /* input,output */
};

struct hdcdrv_cmd_set_service_level {
    int ret;
    int dev_id;
    unsigned long long pid;
    int service_type; /* input */
    int level;        /* input */
};

struct hdcdrv_cmd_server_create {
    int ret;
    int dev_id;       /* input */
    unsigned long long pid;          /* input */
    int service_type; /* input */
};

struct hdcdrv_cmd_server_destroy {
    int ret;
    int dev_id;       /* input */
    unsigned long long pid;
    int service_type; /* input */
};

struct hdcdrv_cmd_accept {
    int ret;
    int dev_id;                      /* input */
    unsigned long long pid;
    int service_type;                /* input */
    int session;                     /* output */
};

struct hdcdrv_cmd_connect {
    int peer_pid;                    /* input */
    int dev_id;                      /* input */
    unsigned long long pid;
    int service_type;                /* input */
    int session;                     /* output */
};

struct hdcdrv_cmd_close {
    int ret;
    int dev_id;
    unsigned long long pid;
    int session;                     /* input */
    unsigned int unique_val;
};

struct hdcdrv_cmd_send {
    int ret;
    int dev_id;
    unsigned long long pid;
    int session;                     /* input */
    void *src_buf;                   /* input */
    int len;                         /* input */
    void *pool_buf;                  /* input */
    unsigned long long pool_addr;    /* input */
    int wait_flag;                   /* input */
    unsigned int timeout;            /* input */
};

struct hdcdrv_cmd_recv_peek {
    int ret;
    int dev_id;
    unsigned long long pid;
    int session;                     /* input */
    int len;                         /* output, if remote session close, this is 0 */
    int wait_flag;                   /* input */
    unsigned int timeout;            /* input */
    int group_flag;                  /* input */
    int count;                       /* output, if remote session close, this is 0 */
};

struct hdcdrv_cmd_recv {
    int ret;
    int dev_id;
    unsigned long long pid;
    int session;                     /* input */
    void *dst_buf;                   /* output */
    int len;                         /* input */
    void *pool_buf;                  /* output */
    int out_len;                     /* output */
    int buf_count;                   /* output, for vhdc used */
    int group_flag;                  /* input */
    void *buf_list[HDCDRV_SESSION_RX_LIST_MAX_PKT];         /* output, for vhdc used in VM */
    unsigned int buf_len[HDCDRV_SESSION_RX_LIST_MAX_PKT];   /* output, for vhdc used in VM */
};

struct hdcdrv_cmd_set_session_owner {
    int ret;
    int dev_id;
    unsigned long long pid;                         /* owner pid */
    unsigned long long ppid;
    int session;                     /* input */
};

struct hdcdrv_cmd_get_session_attr {
    int ret;
    int dev_id;
    unsigned long long pid;
    int cmd_type;                    /* input */
    int session;                     /* input */
    int output;                      /* output */
};

struct hdcdrv_cmd_set_session_timeout {
    int ret;
    int dev_id;
    unsigned long long pid;
    int session;                     /* input */
    struct hdcdrv_timeout timeout;   /* input */
};

struct hdcdrv_cmd_get_uid_stat {
    int ret;
    int dev_id;             /* input, -1 not care */
    unsigned long long pid;
    int session;            /* input, -1 not care */
    unsigned int euid;
    unsigned int uid;
    int root_privilege;
};

struct hdcdrv_cmd_alloc_mem {
    int ret;
    int dev_id;
    unsigned long long pid;
    unsigned int type;      /* input */
    unsigned int len;       /* input */
    unsigned long long va;  /* input */
    unsigned int page_type; /* input */
    int map;                /* input */
};

struct hdcdrv_cmd_free_mem {
    int ret;
    int dev_id;
    unsigned long long pid;
    unsigned int type;          /* input */
    unsigned int len;           /* output */
    unsigned int page_type;     /* output */
    unsigned long long va;      /* input */
};

struct hdcdrv_cmd_fast_send {
    int ret;
    int dev_id;
    unsigned long long pid;
    int session;                      /* input */
    int wait_flag;                    /* input */
    unsigned long long src_data_addr; /* input */
    unsigned long long dst_data_addr; /* input */
    unsigned long long src_ctrl_addr; /* input */
    unsigned long long dst_ctrl_addr; /* input */
    int data_len;                     /* input */
    int ctrl_len;                     /* input */
    unsigned int timeout;             /* input */
};

struct hdcdrv_cmd_fast_recv {
    int ret;
    int dev_id;
    unsigned long long pid;
    int session;                     /* input */
    int wait_flag;                   /* input */
    unsigned int timeout;            /* input */
    unsigned long long data_addr;    /* output */
    unsigned long long ctrl_addr;    /* output */
    int data_len;                    /* output */
    int ctrl_len;                    /* output */
};

struct hdcdrv_cmd_dma_map {
    int ret;
    int dev_id;
    unsigned long long pid;
    unsigned int type;     /* input */
    unsigned long long va; /* input */
};

struct hdcdrv_cmd_dma_unmap {
    int ret;
    int dev_id;
    unsigned long long pid;
    unsigned int type;     /* input */
    unsigned long long va; /* input */
};

struct hdcdrv_cmd_dma_remap {
    int ret;
    int dev_id;
    unsigned long long pid;
    unsigned int type;     /* input */
    unsigned long long va; /* input */
};

struct hdcdrv_cmd_register_mem {
    int ret;
    int dev_id;
    unsigned long long pid;
    unsigned int type;      /* input */
    unsigned int len;       /* input */
    unsigned long long va;  /* input */
    unsigned int flag; /* input */
};

struct hdcdrv_cmd_unregister_mem {
    int ret;
    int dev_id;
    unsigned long long pid;
    unsigned int type;          /* input */
    unsigned int len;           /* output */
    unsigned int page_type;     /* output */
    unsigned long long va;      /* input */
};

struct hdcdrv_cmd_wait_mem {
    int ret;
    int dev_id;
    unsigned long long pid;
    int session;                     /* input */
    int timeout;                     /* input */
    unsigned long long data_addr;    /* output */
    unsigned long long ctrl_addr;    /* output */
    int data_len;                    /* output */
    int ctrl_len;                    /* output */
};

struct hdcdrv_cmd_epoll_alloc_fd {
    int ret;
    int dev_id;
    unsigned long long pid;
    int size;               /* input */
    int epfd;               /* output */
};

struct hdcdrv_cmd_epoll_free_fd {
    int ret;
    int dev_id;
    unsigned long long pid;
    int epfd;               /* input */
};

struct hdcdrv_cmd_epoll_ctl {
    int ret;
    int dev_id;
    unsigned long long pid;
    int epfd;                   /* input */
    int op;                     /* input */
    int para1;                  /* input, service:dev_id, session:session_fd */
    int para2;                  /* input, service:service_type */
    struct hdcdrv_event event;  /* input */
};

struct hdcdrv_cmd_epoll_wait {
    int ret;
    int dev_id;
    unsigned long long pid;
    int epfd;                   /* input */
    int timeout;                /* input */
    int maxevents;              /* input */
    int ready_event;            /* output */
    struct hdcdrv_event *event;  /* output */
    struct hdcdrv_event vevent[HDCDRV_VEPOLL_EVENT_MAX];
};

struct hdcdrv_cmd_get_page_size {
    int ret;
    int dev_id;
    unsigned long long pid;
    unsigned int page_size;
    unsigned int hpage_size;
    unsigned int page_bit;
};

struct hdcdrv_cmd_get_session_info {
    int ret;
    int dev_id;
    unsigned long long pid;
    unsigned int fid;
    int session_fd;
};

union hdcdrv_cmd {
    struct hdcdrv_cmd_common cmd_com;
    struct hdcdrv_cmd_get_peer_dev_id get_peer_dev_id;
    struct hdcdrv_cmd_config config;
    struct hdcdrv_cmd_set_service_level set_level;
    struct hdcdrv_cmd_server_create server_create;
    struct hdcdrv_cmd_server_destroy server_destroy;
    struct hdcdrv_cmd_accept accept;
    struct hdcdrv_cmd_connect conncet;
    struct hdcdrv_cmd_close close;
    struct hdcdrv_cmd_send send;
    struct hdcdrv_cmd_recv_peek recv_peek;
    struct hdcdrv_cmd_recv recv;
    struct hdcdrv_cmd_set_session_owner set_owner;
    struct hdcdrv_cmd_get_session_attr get_session_attr;
    struct hdcdrv_cmd_set_session_timeout set_session_timeout;
    struct hdcdrv_cmd_get_uid_stat get_uid_stat;
    struct hdcdrv_cmd_alloc_mem alloc_mem;
    struct hdcdrv_cmd_free_mem free_mem;
    struct hdcdrv_cmd_fast_send fast_send;
    struct hdcdrv_cmd_fast_recv fast_recv;
    struct hdcdrv_cmd_dma_map dma_map;
    struct hdcdrv_cmd_dma_unmap dma_unmap;
    struct hdcdrv_cmd_dma_remap dma_remap;
    struct hdcdrv_cmd_register_mem register_mem;
    struct hdcdrv_cmd_unregister_mem unregister_mem;
    struct hdcdrv_cmd_wait_mem wait_mem;
    struct hdcdrv_cmd_epoll_alloc_fd epoll_alloc_fd;
    struct hdcdrv_cmd_epoll_free_fd epoll_free_fd;
    struct hdcdrv_cmd_epoll_ctl epoll_ctl;
    struct hdcdrv_cmd_epoll_wait epoll_wait;
    struct hdcdrv_cmd_get_page_size get_page_size;
    struct hdcdrv_cmd_get_session_info get_session_info;
};

#endif
