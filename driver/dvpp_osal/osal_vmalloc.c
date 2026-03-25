#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/version.h>
#include "hi_osal.h"
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#include <asm/pgtable-prot.h>
#include <asm-generic/getorder.h>
#endif // #ifdef AOS_LLVM_BUILD

mem_record_info g_mem_record_info[MAX_MEM_INFO] = {0};
EXPORT_SYMBOL(g_mem_record_info);

hi_u32 g_mem_record_idx = 0;
EXPORT_SYMBOL(g_mem_record_idx);

#ifdef AOS_LLVM_BUILD
#define THISNODE_FLAG 0
#define HIGHUSER_MOVABLE_FLG 0
// milan v1r3 mdc版本有两个内存node, __vmalloc_node从node0上申请不到会尝试从node2申请,
// 当node0内存不足从node2申请时，__GFP_THISNODE宏会严重影响性能
#elif defined(DVPP_MINI_V2_MDC)
#define THISNODE_FLAG 0
#define HIGHUSER_MOVABLE_FLG 0
#else
#define THISNODE_FLAG __GFP_THISNODE
#define HIGHUSER_MOVABLE_FLG GFP_HIGHUSER_MOVABLE
#endif

#ifdef DVPP_VCAST_UTST
#undef THISNODE_FLAG
#define THISNODE_FLAG 0x4000

#undef HIGHUSER_MOVABLE_FLG
#define HIGHUSER_MOVABLE_FLG 0

#endif

static hi_bool g_support_print_alloc_mem_info = HI_FALSE;

void osal_set_print_alloc_mem_info(hi_bool flag)
{
    g_support_print_alloc_mem_info = flag;
}
EXPORT_SYMBOL(osal_set_print_alloc_mem_info);

void *osal_vmalloc_(hi_ulong size, const char *function)
{
    void *addr = HI_NULL;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0)) || defined AOS_LLVM_BUILD
    addr = __vmalloc(size, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT, PAGE_KERNEL);
#else
    addr = __vmalloc(size, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT);
#endif
    g_mem_record_info[g_mem_record_idx].func = (char *)function;
    g_mem_record_info[g_mem_record_idx].kva = addr;
    g_mem_record_info[g_mem_record_idx].uva = HI_NULL;
    g_mem_record_info[g_mem_record_idx].size = size;
    g_mem_record_info[g_mem_record_idx].type = MEM_OP_VMALLOC;
    g_mem_record_idx = (g_mem_record_idx + 1) % MAX_MEM_INFO;
    if (g_support_print_alloc_mem_info == HI_TRUE) {
        HI_TRACE_OSAL(HI_DBG_DEBUG, "osal vmalloc size=%lu\n", size);
    }
    return addr;
}
EXPORT_SYMBOL(osal_vmalloc_);

void osal_vfree_(const void *addr, const char *function)
{
    vfree(addr);
    g_mem_record_info[g_mem_record_idx].func = (char *)function;
    g_mem_record_info[g_mem_record_idx].kva = addr;
    g_mem_record_info[g_mem_record_idx].type = MEM_OP_VFREE;
    g_mem_record_idx = (g_mem_record_idx + 1) % MAX_MEM_INFO;
}
EXPORT_SYMBOL(osal_vfree_);

hi_void *osal_vmalloc_node(hi_ulong size, hi_s32 node_id)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) || (defined AOS_LLVM_BUILD)
    return __vmalloc_node(size, SHMLBA, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT | THISNODE_FLAG| HIGHUSER_MOVABLE_FLG,
        PAGE_KERNEL, VM_USERMAP, node_id, __builtin_return_address(0));
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    /* __vmalloc_node_range not exported in 6.x; use __vmalloc_node (GPL) */
    return __vmalloc_node(size, SHMLBA,
        GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT | THISNODE_FLAG | HIGHUSER_MOVABLE_FLG,
        node_id, __builtin_return_address(0));
#else
    return __vmalloc_node_range(size,
        SHMLBA,
        VMALLOC_START,
        VMALLOC_END,
        GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT | THISNODE_FLAG | HIGHUSER_MOVABLE_FLG,
        PAGE_KERNEL,
        VM_USERMAP,
        node_id,
        __builtin_return_address(0));
#endif
}
EXPORT_SYMBOL(osal_vmalloc_node);

hi_void *osal_vmalloc_hugepage_node(hi_ulong size, hi_s32 node_id)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) || (defined AOS_LLVM_BUILD)
    size = PMD_ALIGN(size);
    return __vmalloc_node(size, 1, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT | THISNODE_FLAG | HIGHUSER_MOVABLE_FLG,
        PAGE_KERNEL, VM_HUGEPAGE | VM_USERMAP, node_id,
        __builtin_return_address(0));
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    size = PMD_ALIGN(size);
    return __vmalloc_node(size, PMD_SIZE,
        GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT | THISNODE_FLAG | HIGHUSER_MOVABLE_FLG,
        node_id, __builtin_return_address(0));
#else
    size = PMD_ALIGN(size);
    return __vmalloc_node_range(size,
        SHMLBA,
        VMALLOC_START,
        VMALLOC_END,
        GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT | THISNODE_FLAG | HIGHUSER_MOVABLE_FLG,
        PAGE_KERNEL,
        VM_HUGEPAGE | VM_USERMAP,
        node_id,
        __builtin_return_address(0));
#endif
}
EXPORT_SYMBOL(osal_vmalloc_hugepage_node);

void *osal_kmalloc_(hi_ulong size, hi_u32 osal_gfp_flag, const char *function)
{
    void *addr = HI_NULL;
    if (osal_gfp_flag == osal_gfp_kernel) {
        addr = kmalloc(size, GFP_KERNEL);
    } else if (osal_gfp_flag == osal_gfp_atomic) {
        addr = kmalloc(size, GFP_ATOMIC);
    } else {
        addr = NULL;
    }
    g_mem_record_info[g_mem_record_idx].func = (char *)function;
    g_mem_record_info[g_mem_record_idx].kva = addr;
    g_mem_record_info[g_mem_record_idx].uva = HI_NULL;
    g_mem_record_info[g_mem_record_idx].size = size;
    g_mem_record_info[g_mem_record_idx].type = MEM_OP_KMALLOC;
    g_mem_record_idx = (g_mem_record_idx + 1) % MAX_MEM_INFO;
    if (g_support_print_alloc_mem_info == HI_TRUE) {
        HI_TRACE_OSAL(HI_DBG_DEBUG, "osal kmalloc size=%lu\n", size);
    }
    return addr;
}
EXPORT_SYMBOL(osal_kmalloc_);

void osal_kfree_(const void *addr, const char *function)
{
    kfree(addr);
    g_mem_record_info[g_mem_record_idx].func = (char *)function;
    g_mem_record_info[g_mem_record_idx].kva = addr;
    g_mem_record_info[g_mem_record_idx].type = MEM_OP_KFREE;
    g_mem_record_idx = (g_mem_record_idx + 1) % MAX_MEM_INFO;
}

EXPORT_SYMBOL(osal_kfree_);

hi_ulong osal_get_free_pages(hi_u32 osal_gfp_flag, hi_u32 order)
{
    gfp_t gfp_mask = (osal_gfp_flag == osal_gfp_kernel) ? GFP_KERNEL : GFP_ATOMIC;
    return __get_free_pages(gfp_mask, order);
}
EXPORT_SYMBOL(osal_get_free_pages);

#ifndef AOS_LLVM_BUILD
hi_ulong osal_get_free_pages_node(hi_s32 nid, hi_u32 osal_gfp_flag, hi_u32 order)
{
    struct page *page;
    gfp_t gfp_mask = (osal_gfp_flag == osal_gfp_kernel) ? GFP_KERNEL : GFP_ATOMIC;
    page = alloc_pages_node(nid, gfp_mask & ~__GFP_HIGHMEM, order);
    if (!page) {
        return 0;
    }
    return (hi_ulong)page_address(page);
}
#else
hi_ulong osal_get_free_pages_node(hi_s32 nid, hi_u32 osal_gfp_flag, hi_u32 order)
{
    return osal_get_free_pages(osal_gfp_flag, order);
}
#endif // AOS_LLVM_BUILD
EXPORT_SYMBOL(osal_get_free_pages_node);

void osal_free_pages(hi_ulong addr, hi_u32 order)
{
    free_pages(addr, order);
}
EXPORT_SYMBOL(osal_free_pages);

hi_u32 osal_get_order(hi_ulong size)
{
    return (hi_u32)get_order(size);
}
EXPORT_SYMBOL(osal_get_order);

#if !defined (DVPP_HLP) && !defined (AOS_LLVM_BUILD)
void* osal_vmalloc_to_page(void *addr)
{
    return vmalloc_to_page(addr);
}
EXPORT_SYMBOL(osal_vmalloc_to_page);

hi_u32 osal_get_page_size(void *pg)
{
    return page_size((struct page *)pg);
}
EXPORT_SYMBOL(osal_get_page_size);

hi_phys_addr_t osal_page_to_phys(hi_void *pg)
{
    return page_to_phys((struct page *)pg);
}
EXPORT_SYMBOL(osal_page_to_phys);

#endif // DVPP_HLP AOS_LLVM_BUILD