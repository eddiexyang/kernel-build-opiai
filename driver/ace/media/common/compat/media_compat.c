#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>

#include "hi_osal.h"
#include "mod_ext.h"
#include "mm_ext.h"
#include "media_mem_file.h"

struct media_compat_alloc {
	struct list_head node;
	hi_u64 phys_addr;
	void *virt_addr;
	hi_ulong size;
};

static DEFINE_MUTEX(g_cmpi_lock);
static umap_module *g_cmpi_modules[MAX_MPP_MODULES];

static DEFINE_MUTEX(g_media_alloc_lock);
static LIST_HEAD(g_media_alloc_list);

static struct media_compat_alloc *media_compat_find_by_phys(hi_u64 phys_addr)
{
	struct media_compat_alloc *entry;

	list_for_each_entry(entry, &g_media_alloc_list, node) {
		if (entry->phys_addr == phys_addr)
			return entry;
	}

	return NULL;
}

static struct media_compat_alloc *media_compat_find_by_virt(const void *virt_addr)
{
	struct media_compat_alloc *entry;

	list_for_each_entry(entry, &g_media_alloc_list, node) {
		if (entry->virt_addr == virt_addr)
			return entry;
	}

	return NULL;
}

static hi_s32 media_compat_alloc_contig(hi_ulong size, hi_u64 *phys_addr, void **virt_addr)
{
	struct media_compat_alloc *entry;
	void *buffer;
	hi_ulong alloc_size;

	if ((phys_addr == NULL) || (virt_addr == NULL) || (size == 0))
		return -EINVAL;

	alloc_size = PAGE_ALIGN(size);
	buffer = alloc_pages_exact(alloc_size, GFP_KERNEL | __GFP_ZERO);
	if (buffer == NULL)
		return -ENOMEM;

	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (entry == NULL) {
		free_pages_exact(buffer, alloc_size);
		return -ENOMEM;
	}

	entry->virt_addr = buffer;
	entry->phys_addr = virt_to_phys(buffer);
	entry->size = alloc_size;

	mutex_lock(&g_media_alloc_lock);
	list_add_tail(&entry->node, &g_media_alloc_list);
	mutex_unlock(&g_media_alloc_lock);

	*virt_addr = buffer;
	*phys_addr = entry->phys_addr;
	return 0;
}

static void media_compat_free_contig_locked(struct media_compat_alloc *entry)
{
	list_del(&entry->node);
	free_pages_exact(entry->virt_addr, entry->size);
	kfree(entry);
}

hi_char *cmpi_get_module_name(hi_mod_id mod_id)
{
	umap_module *mod;

	if ((mod_id < 0) || (mod_id >= MAX_MPP_MODULES))
		return NULL;

	mutex_lock(&g_cmpi_lock);
	mod = g_cmpi_modules[mod_id];
	mutex_unlock(&g_cmpi_lock);

	return mod ? mod->mod_name : NULL;
}
EXPORT_SYMBOL(cmpi_get_module_name);

umap_module *cmpi_get_module_by_id(hi_mod_id mod_id)
{
	umap_module *mod;

	if ((mod_id < 0) || (mod_id >= MAX_MPP_MODULES))
		return NULL;

	mutex_lock(&g_cmpi_lock);
	mod = g_cmpi_modules[mod_id];
	mutex_unlock(&g_cmpi_lock);

	return mod;
}
EXPORT_SYMBOL(cmpi_get_module_by_id);

hi_void *cmpi_get_module_func_by_id(hi_mod_id mod_id)
{
	umap_module *mod = cmpi_get_module_by_id(mod_id);

	return mod ? mod->export_funcs : NULL;
}
EXPORT_SYMBOL(cmpi_get_module_func_by_id);

hi_void cmpi_stop_modules(hi_void)
{
}
EXPORT_SYMBOL(cmpi_stop_modules);

hi_s32 cmpi_query_modules(hi_void)
{
	return 0;
}
EXPORT_SYMBOL(cmpi_query_modules);

hi_s32 cmpi_init_modules(hi_void)
{
	return 0;
}
EXPORT_SYMBOL(cmpi_init_modules);

hi_void cmpi_exit_modules(hi_void)
{
}
EXPORT_SYMBOL(cmpi_exit_modules);

hi_s32 cmpi_register_module(umap_module *mod)
{
	if ((mod == NULL) || (mod->mod_id < 0) || (mod->mod_id >= MAX_MPP_MODULES))
		return -EINVAL;

	mutex_lock(&g_cmpi_lock);
	g_cmpi_modules[mod->mod_id] = mod;
	mutex_unlock(&g_cmpi_lock);

	return 0;
}
EXPORT_SYMBOL(cmpi_register_module);

hi_void cmpi_unregister_module(hi_mod_id mod_id)
{
	if ((mod_id < 0) || (mod_id >= MAX_MPP_MODULES))
		return;

	mutex_lock(&g_cmpi_lock);
	g_cmpi_modules[mod_id] = NULL;
	mutex_unlock(&g_cmpi_lock);
}
EXPORT_SYMBOL(cmpi_unregister_module);

hi_u64 cmpi_mmz_malloc(const hi_char *mmz_name, const hi_char *buf_name, hi_ulong ul_size)
{
	hi_u64 phys_addr = 0;
	void *virt_addr = NULL;

	if (media_compat_alloc_contig(ul_size, &phys_addr, &virt_addr) != 0)
		return 0;

	return phys_addr;
}
EXPORT_SYMBOL(cmpi_mmz_malloc);

hi_void cmpi_mmz_free(hi_u64 phy_addr, hi_void *vir_addr)
{
	struct media_compat_alloc *entry = NULL;

	mutex_lock(&g_media_alloc_lock);
	if (phy_addr != 0)
		entry = media_compat_find_by_phys(phy_addr);
	if ((entry == NULL) && (vir_addr != NULL))
		entry = media_compat_find_by_virt(vir_addr);
	if (entry != NULL)
		media_compat_free_contig_locked(entry);
	mutex_unlock(&g_media_alloc_lock);
}
EXPORT_SYMBOL(cmpi_mmz_free);

hi_s32 cmpi_mmz_malloc_nocache(const hi_char *cp_mmz_name, const hi_char *buf_name,
	hi_u64 *phy_addr, hi_void **pp_vir_addr, hi_ulong ul_len)
{
	return media_compat_alloc_contig(ul_len, phy_addr, pp_vir_addr);
}
EXPORT_SYMBOL(cmpi_mmz_malloc_nocache);

hi_s32 cmpi_mmz_malloc_cached(const hi_char *cp_mmz_name, const hi_char *buf_name,
	hi_u64 *phy_addr, hi_void **pp_vir_addr, hi_ulong ul_len)
{
	return media_compat_alloc_contig(ul_len, phy_addr, pp_vir_addr);
}
EXPORT_SYMBOL(cmpi_mmz_malloc_cached);

hi_s32 cmpi_mmz_acquire_uva(hi_ulong len, hi_void *pp_vir_addr, hi_u64 *phy_addr, pid_t pid)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(cmpi_mmz_acquire_uva);

hi_s32 cmpi_mmz_release_uva(hi_u64 phy_addr, pid_t pid)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(cmpi_mmz_release_uva);

hi_void cmpi_unmap(hi_void *virt_addr)
{
}
EXPORT_SYMBOL(cmpi_unmap);

hi_void *cmpi_remap_nocache(hi_u64 phy_addr, hi_ulong size)
{
	struct media_compat_alloc *entry;
	hi_void *virt = NULL;

	mutex_lock(&g_media_alloc_lock);
	entry = media_compat_find_by_phys(phy_addr);
	if (entry != NULL)
		virt = entry->virt_addr;
	mutex_unlock(&g_media_alloc_lock);

	return virt;
}
EXPORT_SYMBOL(cmpi_remap_nocache);

hi_s32 media_mem_check_sharepool(hi_u64 addr, hi_u32 size)
{
	return HI_FALSE;
}
EXPORT_SYMBOL(media_mem_check_sharepool);

hi_s32 media_mem_file_init(struct media_mem_file *mfile, pid_t pid, hi_s32 chn_id, hi_s32 mod_id)
{
	if (mfile == NULL)
		return -EINVAL;

	memset(mfile, 0, sizeof(*mfile));
	OSAL_INIT_LIST_HEAD(&mfile->list);
	OSAL_INIT_LIST_HEAD(&mfile->lru);
	mfile->clist.first = NULL;
	mfile->pid = pid;
	mfile->chn_id = chn_id;
	mfile->mod_id = mod_id;
	mfile->mm = current ? current->mm : NULL;
	(void)osal_spin_lock_init(&mfile->lock);
	(void)osal_spin_lock_init(&mfile->hl_lock);

	return 0;
}
EXPORT_SYMBOL(media_mem_file_init);

hi_s32 media_mem_malloc_(struct media_mem_file *mfile, const hi_char *name0, const hi_char *name1,
	hi_ulong len, hi_u32 flags, hi_u64 *iova_addr, hi_void **k_virt_addr,
	hi_u32 device_id, const char *function)
{
	return media_compat_alloc_contig(len, iova_addr, k_virt_addr);
}
EXPORT_SYMBOL(media_mem_malloc_);

hi_void media_mem_free_(struct media_mem_file *mfile, hi_ulong len, hi_u64 iova_addr,
	hi_void *k_virt_addr, const char *function)
{
	cmpi_mmz_free(iova_addr, k_virt_addr);
}
EXPORT_SYMBOL(media_mem_free_);

hi_s32 media_mem_acquire_iova(const struct media_mem_file *mfile, const hi_void *u_virt_addr,
	hi_ulong len, hi_u32 flags, hi_void **k_virt_addr, hi_u64 *iova_addr)
{
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(media_mem_acquire_iova);

hi_void media_mem_release_iova(const struct media_mem_file *mfile, hi_ulong len,
	const hi_void *u_virt_addr, const hi_void *k_virt_addr)
{
}
EXPORT_SYMBOL(media_mem_release_iova);

hi_s32 media_mem_region_check(const struct media_mem_file *mfile, hi_u64 iova_addr, hi_u64 len)
{
	return 0;
}
EXPORT_SYMBOL(media_mem_region_check);

hi_void media_mem_file_uninit(struct media_mem_file *mfile)
{
}
EXPORT_SYMBOL(media_mem_file_uninit);

hi_s32 media_kernel_mem_malloc(hi_ulong len, hi_u32 flags, hi_void **k_virt_addr)
{
	hi_u64 iova_addr = 0;

	return media_compat_alloc_contig(len, &iova_addr, k_virt_addr);
}
EXPORT_SYMBOL(media_kernel_mem_malloc);

hi_void media_kernel_mem_free(hi_void *k_virt_addr)
{
	cmpi_mmz_free(0, k_virt_addr);
}
EXPORT_SYMBOL(media_kernel_mem_free);

hi_s32 media_kernel_mem_map(struct media_mem_file *mfile, hi_ulong len,
	const hi_void *k_virt_addr, hi_u64 *iova_addr)
{
	struct media_compat_alloc *entry;

	if ((k_virt_addr == NULL) || (iova_addr == NULL))
		return -EINVAL;

	mutex_lock(&g_media_alloc_lock);
	entry = media_compat_find_by_virt(k_virt_addr);
	if (entry != NULL)
		*iova_addr = entry->phys_addr;
	mutex_unlock(&g_media_alloc_lock);

	return entry ? 0 : -ENOENT;
}
EXPORT_SYMBOL(media_kernel_mem_map);

hi_s32 media_kernel_mem_unmap(struct media_mem_file *mfile, hi_ulong len, hi_u64 iova_addr)
{
	return 0;
}
EXPORT_SYMBOL(media_kernel_mem_unmap);

hi_s32 media_protect_mem_malloc_(struct media_mem_file *mfile, hi_ulong len, hi_u64 *iova_addr,
	hi_void **k_virt_addr, hi_u32 device_id, const hi_char *function)
{
	return media_compat_alloc_contig(len, iova_addr, k_virt_addr);
}
EXPORT_SYMBOL(media_protect_mem_malloc_);

hi_void media_protect_mem_free_(struct media_mem_file *mfile, hi_ulong len, hi_u64 iova_addr,
	hi_void *k_virt_addr, const hi_char *function)
{
	cmpi_mmz_free(iova_addr, k_virt_addr);
}
EXPORT_SYMBOL(media_protect_mem_free_);

hi_void media_protect_mem_release_iova(struct media_mem_file *mfile)
{
}
EXPORT_SYMBOL(media_protect_mem_release_iova);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Minimal media compatibility exports for VDP/TDE/HDMI");
