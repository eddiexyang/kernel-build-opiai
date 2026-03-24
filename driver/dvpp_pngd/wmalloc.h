/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: tde wmalloc
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */

#ifndef WMALLOC_H
#define WMALLOC_H

#include "ot_type.h"
#include "ot_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* CNcomment:
 * Function: wmeminit
 * Description: init memory pool. CNcomment: TDE memory pool initialization
 * Return: success/error code. CNcomment: Success/error code
 */
td_s32 wmeminit(void);

/*
 * Function: wmemterm
 * Description: Deinit memory pool. CNcomment:TDE memory pool initialization
 */
td_void wmemterm(void);

/*
 * Function: wget_kernel_addr
 * Description: get the kernal virtual address of memory. CNcomment: Access to the kernal virtual address of memory
 * Input: user_addr. CNcomment: user address
 * Output: kernel address. CNcomment: kernel address
 */
td_void* wget_kernel_addr(td_phys_addr_t user_addr);

/*
 * Function: wget_user_addr
 * Description: get the user address of memory. CNcomment: To obtain the user address of memory
 * Input: kernel_addr . CNcomment: Pointer to the memory
 * Output:  user address. CNcomment: User address
 */
td_phys_addr_t wget_user_addr(const td_void *kernel_addr);

/*
 * Function: wgetfreenum
 * Description:get the number of the free unit. CNcomment: Memory for the rest of the Unit cell number
 * Return: the number of the free unit. CNcomment: The rest of the unit number
 */
td_u32 wgetfreenum(td_void);

td_s32 wmem_get_poolphy_addr(td_void);
td_s32 wmem_free_poolphy_addr(td_void);

#ifndef TDE_BOOT
const osal_proc_entry_t *wprintinfo(const osal_proc_entry_t *page);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* WMALLOC_H */
