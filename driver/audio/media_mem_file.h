/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2019. All rights reserved.
* Description: dvpp mem device
* Author: Hisilicon multimedia software group
* Create: 2016/10/30
*/

#ifndef _MEDIA_MEM_FILE_H
#define _MEDIA_MEM_FILE_H

#include "hi_osal.h"
#include "hi_debug.h"
#include "dvpp_mem_flag_def.h"

#define ATS_INFO_SIZE 3U

#define media_mem_malloc(mfile, name0, name1, len, flags, iova_addr, k_virt_addr, device_id) \
    media_mem_malloc_((mfile), (name0), (name1), (len), (flags), (iova_addr), (k_virt_addr), (device_id), __FUNCTION__)

#define media_mem_free(mfile, len, iova_addr, k_virt_addr) \
    media_mem_free_((mfile), (len), (iova_addr), (k_virt_addr), __FUNCTION__)

#define media_protect_mem_malloc(mfile, len, iova_addr, k_virt_addr, device_id) \
    media_protect_mem_malloc_((mfile), (len), (iova_addr), (k_virt_addr), (device_id), __FUNCTION__)

#define media_protect_mem_free(mfile, len, iova_addr, k_virt_addr) \
    media_protect_mem_free_((mfile), (len), (iova_addr), (k_virt_addr), __FUNCTION__)

struct media_mem_device;
struct media_mem_driver;

struct media_mem_file {
    struct media_mem_device *mdevice;

    // list entry of list in dvpp mem device
    struct osal_list_head list;

    osal_spinlock_t lock;

    hi_u64 total_mem_kernel; // 单位：B
    hi_u64 total_mem_user;

    hi_u64 total_max_mem;

    hi_u32 bo_number;

    // memory driver for alloc/mmap/share
    struct media_mem_driver *driver;

    osal_spinlock_t hl_lock;

    hi_s32 chn_id;
    hi_s32 mod_id;

    // process id
    pid_t pid;
    void* mm;

    // iova address type
    enum dvpp_iova_addr_type addr_type;

    // hlist head for client memory bo
    struct osal_hlist_head clist;

    // list head for kernel bo, the list should be stable
    struct osal_list_head lru;

    // share pool group id
    hi_s32 sp_group_id;

    // iova地址块索引,每块1M
    hi_u32 protect_iova_blk_idx;
};

typedef enum {
    MEM_TYPE_XPC = 0, // aoscore场景的xpc内存
    MEM_TYPE_SVM_SP_0_32G_VENC, // venc模块svm或sharepool内存,低位地址0-32G
    MEM_TYPE_SVM_SP_32_64G_VENC, // venc模块svm或sharepool内存,低位地址32-64G
    MEM_TYPE_SVM_SP_0_32G_VDEC, // vdec模块svm或sharepool内存,低位地址0-32G
    MEM_TYPE_SVM_SP_32_64G_VDEC, // vdec模块svm或sharepool内存,低位地址32-64G
    MEM_TYPE_BUTT,
} input_mem_type;

hi_s32 media_mem_check_sharepool(hi_u64 addr, hi_u32 size);

hi_s32 media_mem_file_init(struct media_mem_file *mfile,
                          pid_t pid,
                          hi_s32 chn_id,
                          hi_s32 mod_id);

// create bo internal
hi_s32 media_mem_malloc_(struct media_mem_file *mfile,
                        const hi_char *name0,
                        const hi_char *name1,
                        hi_ulong len,
                        hi_u32 flags,
                        hi_u64 *iova_addr,
                        hi_void **k_virt_addr,
                        hi_u32 device_id,
                        const char *function);

hi_void media_mem_free_(struct media_mem_file *mfile,
                       hi_ulong len,
                       hi_u64 iova_addr, // iova_addr is not available after alloc
                       hi_void *k_virt_addr, // k_virt_addr is avialable for contiguish and discrete memory
                       const char *function);

// acquire iova from client memory
hi_s32 media_mem_acquire_iova(const struct media_mem_file *mfile,
                             const hi_void *u_virt_addr,
                             hi_ulong len,
                             hi_u32 flags,
                             hi_void **k_virt_addr,
                             hi_u64 *iova_addr);

// release iova from client memory
hi_void media_mem_release_iova(const struct media_mem_file *mfile,
                              hi_ulong len,
                              const hi_void *u_virt_addr,
                              const hi_void *k_virt_addr);

// check address area
hi_s32 media_mem_region_check(const struct media_mem_file *mfile, hi_u64 iova_addr, hi_u64 len);

hi_void media_mem_file_uninit(struct media_mem_file *mfile);
// add for gdc and vgs
hi_s32 media_kernel_mem_malloc(hi_ulong len, hi_u32 flags, hi_void **k_virt_addr);
hi_void media_kernel_mem_free(hi_void *k_virt_addr);
hi_s32 media_kernel_mem_map(struct media_mem_file *mfile,
                           hi_ulong len,
                           const hi_void *k_virt_addr,
                           hi_u64 *iova_addr);
hi_s32 media_kernel_mem_unmap(struct media_mem_file *mfile,
                             hi_ulong len,
                             hi_u64 iova_addr);
hi_s32 media_protect_mem_malloc_(struct media_mem_file *mfile,
    hi_ulong len, hi_u64 *iova_addr, hi_void **k_virt_addr, hi_u32 device_id, const hi_char *function);

hi_void media_protect_mem_free_(struct media_mem_file *mfile,
    hi_ulong len, hi_u64 iova_addr, hi_void *k_virt_addr, const hi_char *function);

hi_void media_protect_mem_release_iova(struct media_mem_file *mfile);

// 获取是否支持内核态页表的功能
static inline hi_bool dvpp_is_support_split_pgtable(hi_void)
{
#ifdef DVPP_HLP
    // 目前helper无法通过chip_type判断，只能通过版本宏区分
    return HI_FALSE;
#else
    return HI_TRUE;
#endif
}

// 将原始的iova转换成对应滑窗需要的地址
static inline hi_u64 dvpp_trans_ats_addr(input_mem_type mem_type, hi_u64 iova, hi_u64 ats_base, hi_u64 start_low_addr)
{
    // 不支持内核态页表的版本不转换地址
    if ((dvpp_is_support_split_pgtable() == HI_TRUE) || (mem_type == MEM_TYPE_XPC)) {
        return (iova - (((ats_base - start_low_addr) >> 20U) << 20U));
    } else {
        return iova;
    }
}

// 获取各类型地址对应滑窗的信息
static inline hi_void dvpp_get_iova_ats_info(input_mem_type mem_type,
    hi_u64 *ats_base, hi_u64 *start_low_addr, hi_u64 *ats_len)
{
    // ats_info保存各类型地址滑窗的信息,一维表示不同的输入内存类型
    // 二维表示滑窗的基地址、低位起始地址，窗口的大小
    static hi_u64 ats_info[MEM_TYPE_BUTT][ATS_INFO_SIZE] = {
        {0xe7fbfffff000U,     0xa00000000U, 0x400000000U},
        {0xffff000000000000U, 0x800000000U, 0x200000000U},
        {0xffff000000000000U, 0x0U,         0x200000000U},
        {0xffff000200000000U, 0x800000000U, 0x200000000U},
        {0xffff000200000000U, 0x0U,         0x200000000U}};
    if ((mem_type < MEM_TYPE_XPC) || (mem_type >= MEM_TYPE_BUTT)) {
        return;
    }
    if (ats_base != HI_NULL) {
        *ats_base = ats_info[mem_type][0];
    }
    if (start_low_addr != HI_NULL) {
        *start_low_addr = ats_info[mem_type][1U];
    }
    if (ats_len != HI_NULL) {
        *ats_len = ats_info[mem_type][2U];
    }
}

#endif // end of _MEDIA_MEM_FILE_H
