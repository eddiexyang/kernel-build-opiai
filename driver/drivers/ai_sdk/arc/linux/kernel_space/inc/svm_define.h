/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-08-13
 */

#ifndef SVM_DEFINE_H__
#define SVM_DEFINE_H__
#include <stdbool.h>

#define BYTES_PER_KB 1024ul

#define DEVMM_MAX_NUMA_NUM_OF_PER_DEV 32U

#define DAVINCI_SVM_SUB_MODULE_NAME "SVM"
#define DAVINCI_SVM_AGENT_SUB_MODULE_NAME "SVM_AGENT"

#define DV_ADVISE_DDR 0x0001
#define DV_ADVISE_HBM 0x0002
#define DV_ADVISE_HUGEPAGE 0x0004
#define DV_ADVISE_POPULATE 0x0008
#define DV_ADVISE_P2P_HBM 0x0010
#define DV_ADVISE_P2P_DDR 0x0020
#define DV_ADVISE_LOCK_DEV 0x0080
#define DV_ADVISE_CONTINUTY 0x0100
#define DV_ADVISE_HOST 0x0200
#define DV_ADVISE_TS_DDR 0x0400
#define DV_ADVISE_PERSISTENT 0x0800
#define DV_ADVISE_READONLY 0x1000

#define DV_ADVISE_MODULE_ID_BIT 24
#define DV_ADVISE_MODULE_ID_MASK 0xff

#define AGENT_ADVISE_MEM_GET 0x0001
#define AGENT_ADVISE_MEM_PUT 0x0002

#define MAX_CONTINUTY_PHYS_SIZE 0x400000ULL /* 4M */

#define DEVMM_MAX_VF_NUM 32

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
/*
 * In 910B sriov/mdev:
 * devid = pf_id * MAX_VF_NUM(16) + (vfid - 1) + 100
 * pf:     devid 0   ~ devid 63
 * vf:     devid 100 ~ devid 1123
 * rsv:    devid 64  ~ devid 99
 *
 * device devid:
 * pf:     devid 0   ~ devid 31
 * vf:     devid 32  ~ devid 63
 */
#define DEVMM_MAX_DEVICE_NUM 1124
#define DEVMM_MAX_AGENTMM_DEVICE_NUM 64
#define DEVMM_MAX_PHY_DEVICE_NUM 64
#define DEVMM_INVALID_DEVICE_PHYID DEVMM_MAX_DEVICE_NUM
#define SVM_DEVICE_SIDE_AGENT_NUM DEVMM_MAX_DEVICE_NUM
#define SVM_HOST_SIDE_AGENT_NUM 1
#define SVM_MAX_AGENT_NUM DEVMM_MAX_DEVICE_NUM
#else
#define DEVMM_MAX_DEVICE_NUM 64
#define DEVMM_MAX_AGENTMM_DEVICE_NUM 4
#define DEVMM_MAX_PHY_DEVICE_NUM DEVMM_MAX_DEVICE_NUM
#define DEVMM_INVALID_DEVICE_PHYID 0xff
#define SVM_DEVICE_SIDE_AGENT_NUM DEVMM_MAX_DEVICE_NUM
#define SVM_HOST_SIDE_AGENT_NUM 1
#define SVM_MAX_AGENT_NUM (SVM_DEVICE_SIDE_AGENT_NUM + SVM_HOST_SIDE_AGENT_NUM)
#endif

#define SVM_HOST_AGENT_ID 64
#define DEVMM_MAX_VMA_NUM   (DEVMM_MAX_PHY_DEVICE_NUM + 2) /* 64 dvpp + 1 read + 1 normal */
#define DEVMM_NORMAL_MAX_VMA_NUM 3 /* vma0: dvpp mem, vma1 read  vma2: normal mem */

#define DEVMM_SVM_DEV_NAME "devmm_svm"
#define DEVMM_SVM_AGENT_DEV_NAME "devmm_svm_agent"
#define DEVMM_SVM_DEV_PATH "/dev/"
#ifndef CFG_ENABLE_ASAN
/* orther addr not test, dev mmap fail because program segments loading random */
#define DEVMM_SVM_MEM_START 0x100000000000ULL
#else
#define DEVMM_SVM_MEM_START 0x210000000000ULL /* device asan 0x100000000000ULL mmap fail */
#endif
#define DEVMM_DEV_MAPPED_RANGE 37   /* 128G */
#define DEVMM_MAPPEDSZ_PER_DEV (1UL << DEVMM_DEV_MAPPED_RANGE)                 /* 128G */
#define DEVMM_MAX_MAPPED_RANGE (DEVMM_MAPPEDSZ_PER_DEV * DEVMM_MAX_PHY_DEVICE_NUM) /* 8T */
#define DEVMM_MAX_ALLOC_PAGE_NUM (1ULL << 29) /* the max page num of ioctl alloc */

#define DEVMM_HEAP_RANGE 30
#define DEVMM_HEAP_SIZE (1UL << DEVMM_HEAP_RANGE) /* 1G */
#define DEVMM_MAX_HEAP_NUM (DEVMM_MAX_MAPPED_RANGE >> DEVMM_HEAP_RANGE)
#define DEVMM_SVM_MEM_SIZE DEVMM_MAX_MAPPED_RANGE

/* dvpp engine addressing range split in two, one for drv, one for os share pool */
#define DEVMM_TWICE_DVPP_HEAP_SIZE 2
#define DEVMM_MAX_HEAP_MEM_FOR_DVPP_4G 0x100000000Ul /* 4G */
#define DEVMM_MAX_HEAP_MEM_FOR_DVPP_16G 0x400000000Ul /* 16G */
#define DEVMM_DVPP_HEAP_MAX_SIZE DEVMM_MAX_HEAP_MEM_FOR_DVPP_16G
#define DEVMM_DVPP_HEAP_RESERVATION_SIZE 0x800000000Ul /* 2 * max */
#define DEVMM_DVPP_HEAP_TOTAL_SIZE 0x20000000000Ul /* 2 * max * DEVMM_MAX_DEVICE_NUM */

#define DEVMM_READ_ONLY_HEAP_SIZE 0x100000000Ul /* 4G */
#define DEVMM_READ_ONLY_HEAP_TOTAL_SIZE 0x4000000000Ul /* 64 * 4G */

#define DEVMM_READ_ONLY_ADDR_START (DEVMM_SVM_MEM_START + DEVMM_DVPP_HEAP_TOTAL_SIZE)
#define DEVMM_READ_ONLY_ADDR_END (DEVMM_SVM_MEM_START + DEVMM_DVPP_HEAP_TOTAL_SIZE + \
    DEVMM_READ_ONLY_HEAP_TOTAL_SIZE - 1)

#define DEVMM_NON_RESERVATION_HEAP_ADDR_START (DEVMM_SVM_MEM_START + DEVMM_DVPP_HEAP_TOTAL_SIZE + \
    DEVMM_READ_ONLY_HEAP_TOTAL_SIZE)

#define DEVMM_INVALID_ADDR 0UL
#define DEVMM_INVALID_ADDR2 1UL /* for ioctl err */

#define DEVMM_TRUE 1UL
#define DEVMM_FALSE 0UL

#define DEVMM_CONVERT_64M_SIZE 0x4000000UL
#define DEVMM_CONVERT_128M_SIZE 0x8000000UL
#define DEVMM_CONVERT_DMA_DEPTH 0x8000UL
#define DEVMM_CONVERT2D_HEIGHT_MAX 0x500000UL /* 5M */

#define DEVMM_ASYNC_CPY_FINISH_VALUE 0x5a5a5a5aul
#define DEVMM_ASYNC_CPY_ERR_VALUE (-18) /*  EXDEV Cross-device link */

#define DEVMM_SHARE_MEM_MAX_PID_CNT 32768 /* 64 process * 512 server node */

/*
 * device:
 * setup device:
 * UNINITED->h2d set POLLING->polled wateup thread set POLLED->forked thread set dev pid & PRE_INITING
 * ->mmap set INITING->ioctl set INITED
 * user drvMemInitSvmDevice:
 * UNINITED->ioctl set pid & PRE_INITING->mmap set INITING->ioctl set INITED
 *
 * host:
 * UNINITED->mmap set pid& INITING->ioctl set INITED
 */
#define DEVMM_SVM_UNINITED_FLAG 0
#define DEVMM_SVM_PRE_INITING_FLAG 3
#define DEVMM_SVM_INITING_FLAG 4
#define DEVMM_SVM_RELEASE_FLAG 5
#define DEVMM_SVM_PRE_INITED_FLAG 0xAABBCCDD
#define DEVMM_SVM_INITED_FLAG 0xFFAACCEE
#define DEVMM_SVM_UNMAP_FLAG 0xAADDCC

#define DEVMM_SVM_INVALID_PID (-1)
#define DEVMM_SVM_INVALID_INDEX (-1)

enum devmm_endpoint_type {
    DEVMM_END_HOST = 0x0,
    DEVMM_END_DEVICE,
    DEVMM_END_NUM
};

enum devmm_side_type {
    DEVMM_SIDE_MASTER = 0,
    DEVMM_SIDE_DEVICE_AGENT = 1,
    DEVMM_SIDE_HOST_AGENT = 2,
    DEVMM_SIDE_MAX
};

enum devmm_mem_type {
    DEVMM_HBM_MEM = 0x0,
    DEVMM_DDR_MEM,
    DEVMM_P2P_HBM_MEM,
    DEVMM_P2P_DDR_MEM,
    DEVMM_TS_DDR_MEM,
    DEVMM_MEM_TYPE_MAX,
};

enum devmm_page_type {
    DEVMM_NORMAL_PAGE_TYPE = 0x0,
    DEVMM_HUGE_PAGE_TYPE,
    DEVMM_PAGE_TYPE_MAX
};

enum devmm_copy_direction {
    DEVMM_COPY_HOST_TO_HOST,
    DEVMM_COPY_HOST_TO_DEVICE,
    DEVMM_COPY_DEVICE_TO_HOST,
    DEVMM_COPY_DEVICE_TO_DEVICE,
    DEVMM_COPY_INVILED_DIRECTION
};

#define DEVMM_HEAP_ENABLE (0ul)
#define DEVMM_HEAP_DISABLE (1ul)

#define DEVMM_HEAP_PINNED_HOST (0xEFEF0001UL)
#define DEVMM_HEAP_HUGE_PAGE (0xEFEF0002UL)
#define DEVMM_HEAP_CHUNK_PAGE (0xEFEF0003UL)   // page_size is exchanged svm page_size

#define DEVMM_HEAP_IDLE (0xEFEFABCDUL)
#define DEVMM_HEAP_OVERSIZED (0xEFEFAAAAUL)

enum devmm_heap_sub_type {
    SUB_SVM_TYPE = 0x0,     /* user mode page is same as kernel page, huge or chunk. the same as MEM_SVM_VAL */
    SUB_DEVICE_TYPE = 0x1,  /* user mode page is same as kernel page, just huge. the same as MEM_DEV_VAL */
    SUB_HOST_TYPE = 0x2,   /* user mode page is same as kernel page just chunk. the same as MEM_HOST_VAL */
    SUB_DVPP_TYPE = 0x3,   /* kernel page is huge, user mode page is chunk. the same as MEM_DVPP_VAL */
    SUB_READ_ONLY_TYPE = 0x4,  /* kernel page is huge, user mode page is chunk. MEM_DEV_VAL */
    SUB_RESERVE_TYPE = 0X5,    /* For halMemAddressReserve */
    SUB_MAX_TYPE
};

#define DEVMM_COPY_LEN_CNT0 (0)
#define DEVMM_COPY_LEN_CNT1 (1)
#define DEVMM_COPY_LEN_CNT2 (2)
#define DEVMM_COPY_LEN_CNT3 (3)
#define DEVMM_COPY_LEN_CNT4 (4)
#define DEVMM_COPY_LEN_CNT32 (32)

/* process normal exit and annormal exit flags */
#define DEVMM_SVM_NORMAL_EXITED_FLAG (0xEEEEEEEE)
#define DEVMM_SVM_ABNORMAL_EXITED_FLAG (0)

#define SVM_MEM_HANDLE_NORMAL_TYPE 0
#define SVM_MEM_HANDLE_EXPORT_TYPE 1
#define SVM_MEM_HANDLE_IMPORT_TYPE 2
#define SVM_MEM_HANDLE_SHARE_TYPE  3

static inline bool devmm_va_is_in_svm_range(unsigned long long va)
{
    return ((va >= DEVMM_SVM_MEM_START) && (va < DEVMM_SVM_MEM_START + DEVMM_SVM_MEM_SIZE));
}

#endif
