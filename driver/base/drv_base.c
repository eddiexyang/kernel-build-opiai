#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "hi_type.h"
#include "osal_list.h"

#define MAX_MPP_MODULES 70
#define VERSION_MAGIC 20190131
#define MAX_MOD_NAME 16
#define HI_DVPP_DBG_DEBUG 7

typedef hi_s32 hi_mod_id;

typedef enum {
	MOD_NOTICE_STOP = 0x11,
} mod_notice_id;

typedef enum {
	MOD_STATE_FREE = 0x11,
	MOD_STATE_BUSY = 0x22,
	MOD_STATE_BYPASS = 0x33,
} mod_state;

typedef enum {
	MOD_REF_GET = 0,
	MOD_REF_PUT,
} module_ref_ctrl;

typedef hi_s32 fn_mod_init(hi_void *);
typedef hi_void fn_mod_exit(hi_void);
typedef hi_void fn_mod_bind(hi_void);
typedef hi_void fn_mod_ref_ctrl(module_ref_ctrl ref_ctrl);
typedef hi_void fn_mod_notify(mod_notice_id notice_id);
typedef hi_void fn_mod_query_state(mod_state *state);
typedef hi_u32 fn_mod_ver_checker(hi_void);

typedef struct {
	struct osal_list_head list;
	hi_char mod_name[MAX_MOD_NAME];
	hi_mod_id mod_id;
	fn_mod_init *pfn_init;
	fn_mod_exit *pfn_exit;
	fn_mod_bind *pfn_bind;
	fn_mod_query_state *pfn_query_state;
	fn_mod_notify *pfn_notify;
	fn_mod_ver_checker *pfn_ver_checker;
	fn_mod_ref_ctrl *pfn_ref_ctrl;
	hi_bool inited;
	hi_void *export_funcs;
	hi_void *data;
	hi_char *version;
	hi_bool has_get_ref;
} umap_module;

typedef struct osal_spinlock {
	void *lock;
} osal_spinlock_t;

enum dvpp_iova_addr_type {
	dvpp_addr_phy,
	dvpp_addr_virt,
	dvpp_addr_iova,
};

struct media_mem_device;
struct media_mem_driver;

struct media_mem_file {
	struct media_mem_device *mdevice;
	struct osal_list_head list;
	osal_spinlock_t lock;
	hi_u64 total_mem_kernel;
	hi_u64 total_mem_user;
	hi_u64 total_max_mem;
	hi_u32 bo_number;
	struct media_mem_driver *driver;
	osal_spinlock_t hl_lock;
	hi_s32 chn_id;
	hi_s32 mod_id;
	pid_t pid;
	void *mm;
	enum dvpp_iova_addr_type addr_type;
	struct osal_hlist_head clist;
	struct osal_list_head lru;
	hi_s32 sp_group_id;
	hi_u32 protect_iova_blk_idx;
};

extern hi_s32 osal_spin_lock_init(osal_spinlock_t *lock);
extern void osal_spin_lock_destroy(osal_spinlock_t *lock);
extern hi_bool osal_is_sharepool_addr(hi_ulong addr);
extern hi_s32 osal_mg_sp_id_of_current(void);
extern void osal_mmget(void *mm);
extern void osal_mmput(void *mm);

struct media_compat_alloc {
	struct list_head node;
	hi_u64 phys_addr;
	void *virt_addr;
	hi_ulong size;
};

static DEFINE_MUTEX(g_cmpi_lock);
static umap_module g_cmpi_modules[MAX_MPP_MODULES];
static hi_u8 g_log_level[MAX_MPP_MODULES];
static hi_u32 g_module_debug_log_level = ~0U;

static DEFINE_MUTEX(g_media_alloc_lock);
static LIST_HEAD(g_media_alloc_list);

static bool cmpi_mod_id_valid(hi_mod_id mod_id)
{
	return (mod_id >= 0) && (mod_id < MAX_MPP_MODULES);
}

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

static void cmpi_reset_module_slot(umap_module *slot)
{
	memset(slot, 0, sizeof(*slot));
}

static hi_s32 cmpi_call_module_init(umap_module *mod)
{
	if (mod->pfn_init == NULL)
		return HI_FAILURE;

	return mod->pfn_init(mod->data);
}

hi_char *cmpi_get_module_name(hi_mod_id mod_id)
{
	hi_char *name = NULL;

	if (!cmpi_mod_id_valid(mod_id))
		return NULL;

	mutex_lock(&g_cmpi_lock);
	if (g_cmpi_modules[mod_id].inited == HI_TRUE)
		name = g_cmpi_modules[mod_id].mod_name;
	mutex_unlock(&g_cmpi_lock);

	return name;
}
EXPORT_SYMBOL(cmpi_get_module_name);

umap_module *cmpi_get_module_by_id(hi_mod_id mod_id)
{
	umap_module *mod = NULL;

	if (!cmpi_mod_id_valid(mod_id))
		return NULL;

	mutex_lock(&g_cmpi_lock);
	if (g_cmpi_modules[mod_id].inited == HI_TRUE)
		mod = &g_cmpi_modules[mod_id];
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
	hi_s32 i;

	mutex_lock(&g_cmpi_lock);
	for (i = 0; i < MAX_MPP_MODULES; i++) {
		if ((g_cmpi_modules[i].inited == HI_TRUE) &&
		    (g_cmpi_modules[i].pfn_notify != NULL))
			g_cmpi_modules[i].pfn_notify(MOD_NOTICE_STOP);
	}
	mutex_unlock(&g_cmpi_lock);
}
EXPORT_SYMBOL(cmpi_stop_modules);

hi_s32 cmpi_query_modules(hi_void)
{
	hi_s32 i;
	mod_state state = MOD_STATE_FREE;

	mutex_lock(&g_cmpi_lock);
	for (i = 0; i < MAX_MPP_MODULES; i++) {
		if ((g_cmpi_modules[i].inited != HI_TRUE) ||
		    (g_cmpi_modules[i].pfn_query_state == NULL))
			continue;

		state = MOD_STATE_FREE;
		g_cmpi_modules[i].pfn_query_state(&state);
		if ((state != MOD_STATE_FREE) && (state != MOD_STATE_BYPASS)) {
			mutex_unlock(&g_cmpi_lock);
			return HI_FAILURE;
		}
	}
	mutex_unlock(&g_cmpi_lock);

	return HI_SUCCESS;
}
EXPORT_SYMBOL(cmpi_query_modules);

hi_s32 cmpi_init_modules(hi_void)
{
	hi_s32 i;
	hi_s32 ret;

	mutex_lock(&g_cmpi_lock);
	for (i = 0; i < MAX_MPP_MODULES; i++) {
		if ((g_cmpi_modules[i].pfn_init == NULL) ||
		    (g_cmpi_modules[i].inited == HI_TRUE))
			continue;

		ret = cmpi_call_module_init(&g_cmpi_modules[i]);
		if (ret != HI_SUCCESS) {
			mutex_unlock(&g_cmpi_lock);
			return ret;
		}
		g_cmpi_modules[i].inited = HI_TRUE;
	}
	mutex_unlock(&g_cmpi_lock);

	return HI_SUCCESS;
}
EXPORT_SYMBOL(cmpi_init_modules);

hi_void cmpi_exit_modules(hi_void)
{
	hi_s32 i;

	mutex_lock(&g_cmpi_lock);
	for (i = 0; i < MAX_MPP_MODULES; i++) {
		if (g_cmpi_modules[i].inited != HI_TRUE)
			continue;

		if (g_cmpi_modules[i].pfn_exit != NULL)
			g_cmpi_modules[i].pfn_exit();
		if ((g_cmpi_modules[i].has_get_ref == HI_TRUE) &&
		    (g_cmpi_modules[i].pfn_ref_ctrl != NULL))
			g_cmpi_modules[i].pfn_ref_ctrl(MOD_REF_PUT);
		g_cmpi_modules[i].inited = HI_FALSE;
	}
	mutex_unlock(&g_cmpi_lock);
}
EXPORT_SYMBOL(cmpi_exit_modules);

hi_s32 cmpi_register_module(umap_module *mod)
{
	hi_s32 ret;

	if ((mod == NULL) || !cmpi_mod_id_valid(mod->mod_id) ||
	    (mod->pfn_init == NULL) || (mod->pfn_exit == NULL) ||
	    (mod->pfn_ver_checker == NULL))
		return HI_FAILURE;

	if (mod->pfn_ver_checker() != VERSION_MAGIC)
		return HI_FAILURE;

	ret = cmpi_call_module_init(mod);
	if (ret != HI_SUCCESS)
		return ret;

	mutex_lock(&g_cmpi_lock);
	if (g_cmpi_modules[mod->mod_id].inited == HI_TRUE) {
		mutex_unlock(&g_cmpi_lock);
		if (mod->pfn_exit != NULL)
			mod->pfn_exit();
		return HI_FAILURE;
	}

	memcpy(&g_cmpi_modules[mod->mod_id], mod, sizeof(*mod));
	g_cmpi_modules[mod->mod_id].inited = HI_TRUE;
	mutex_unlock(&g_cmpi_lock);

	return HI_SUCCESS;
}
EXPORT_SYMBOL(cmpi_register_module);

hi_void cmpi_unregister_module(hi_mod_id mod_id)
{
	if (!cmpi_mod_id_valid(mod_id))
		return;

	mutex_lock(&g_cmpi_lock);
	if (g_cmpi_modules[mod_id].inited == HI_TRUE &&
	    g_cmpi_modules[mod_id].pfn_exit != NULL)
		g_cmpi_modules[mod_id].pfn_exit();
	cmpi_reset_module_slot(&g_cmpi_modules[mod_id]);
	mutex_unlock(&g_cmpi_lock);
}
EXPORT_SYMBOL(cmpi_unregister_module);

hi_s32 cmpi_log_init(hi_u32 log_buf_len)
{
	(void)log_buf_len;
	return HI_SUCCESS;
}
EXPORT_SYMBOL(cmpi_log_init);

hi_void cmpi_log_exit(hi_void)
{
}
EXPORT_SYMBOL(cmpi_log_exit);

hi_s32 hi_chk_log_level(hi_s32 level, hi_mod_id mod_id)
{
	if (!cmpi_mod_id_valid(mod_id))
		return HI_FALSE;

	return (g_log_level[mod_id] >= level) ? HI_TRUE : HI_FALSE;
}
EXPORT_SYMBOL(hi_chk_log_level);

hi_s32 hi_chk_debug_log_level(hi_u32 mod_id, hi_u32 idx)
{
	if ((mod_id + idx) > 0x1f)
		return HI_FALSE;

	return (g_module_debug_log_level >> (mod_id + idx)) & 0x1;
}
EXPORT_SYMBOL(hi_chk_debug_log_level);

hi_u64 cmpi_mmz_malloc(const hi_char *mmz_name, const hi_char *buf_name, hi_ulong ul_size)
{
	hi_u64 phys_addr = 0;
	void *virt_addr = NULL;

	(void)mmz_name;
	(void)buf_name;
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
	(void)cp_mmz_name;
	(void)buf_name;
	return media_compat_alloc_contig(ul_len, phy_addr, pp_vir_addr);
}
EXPORT_SYMBOL(cmpi_mmz_malloc_nocache);

hi_s32 cmpi_mmz_malloc_cached(const hi_char *cp_mmz_name, const hi_char *buf_name,
	hi_u64 *phy_addr, hi_void **pp_vir_addr, hi_ulong ul_len)
{
	(void)cp_mmz_name;
	(void)buf_name;
	return media_compat_alloc_contig(ul_len, phy_addr, pp_vir_addr);
}
EXPORT_SYMBOL(cmpi_mmz_malloc_cached);

hi_s32 cmpi_mmz_acquire_uva(hi_ulong len, hi_void *pp_vir_addr, hi_u64 *phy_addr, pid_t pid)
{
	(void)len;
	(void)pp_vir_addr;
	(void)phy_addr;
	(void)pid;
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(cmpi_mmz_acquire_uva);

hi_s32 cmpi_mmz_release_uva(hi_u64 phy_addr, pid_t pid)
{
	(void)phy_addr;
	(void)pid;
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL(cmpi_mmz_release_uva);

hi_void *cmpi_remap_nocache(hi_u64 phy_addr, hi_ulong size)
{
	struct media_compat_alloc *entry;
	hi_void *virt = NULL;

	(void)size;
	mutex_lock(&g_media_alloc_lock);
	entry = media_compat_find_by_phys(phy_addr);
	if (entry != NULL)
		virt = entry->virt_addr;
	mutex_unlock(&g_media_alloc_lock);

	return virt;
}
EXPORT_SYMBOL(cmpi_remap_nocache);

hi_void cmpi_unmap(hi_void *virt_addr)
{
	(void)virt_addr;
}
EXPORT_SYMBOL(cmpi_unmap);

hi_s32 media_mem_check_sharepool(hi_u64 addr, hi_u32 size)
{
	(void)size;
	return osal_is_sharepool_addr((hi_ulong)addr) ? HI_TRUE : HI_FALSE;
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
	mfile->addr_type = dvpp_addr_iova;
	mfile->mm = current ? current->mm : NULL;
	mfile->sp_group_id = osal_mg_sp_id_of_current();
	if (mfile->mm != NULL)
		osal_mmget(mfile->mm);
	(void)osal_spin_lock_init(&mfile->lock);
	(void)osal_spin_lock_init(&mfile->hl_lock);

	return HI_SUCCESS;
}
EXPORT_SYMBOL(media_mem_file_init);

hi_s32 media_mem_malloc_(struct media_mem_file *mfile, const hi_char *name0, const hi_char *name1,
	hi_ulong len, hi_u32 flags, hi_u64 *iova_addr, hi_void **k_virt_addr,
	hi_u32 device_id, const char *function)
{
	(void)mfile;
	(void)name0;
	(void)name1;
	(void)flags;
	(void)device_id;
	(void)function;
	return media_compat_alloc_contig(len, iova_addr, k_virt_addr);
}
EXPORT_SYMBOL(media_mem_malloc_);

hi_void media_mem_free_(struct media_mem_file *mfile, hi_ulong len, hi_u64 iova_addr,
	hi_void *k_virt_addr, const char *function)
{
	(void)mfile;
	(void)len;
	(void)function;
	cmpi_mmz_free(iova_addr, k_virt_addr);
}
EXPORT_SYMBOL(media_mem_free_);

hi_s32 media_mem_acquire_iova(const struct media_mem_file *mfile, const hi_void *u_virt_addr,
	hi_ulong len, hi_u32 flags, hi_void **k_virt_addr, hi_u64 *iova_addr)
{
	(void)mfile;
	(void)len;
	(void)flags;

	if ((u_virt_addr == NULL) || (k_virt_addr == NULL) || (iova_addr == NULL))
		return -EINVAL;

	*k_virt_addr = (hi_void *)u_virt_addr;
	*iova_addr = (hi_u64)(uintptr_t)u_virt_addr;
	return HI_SUCCESS;
}
EXPORT_SYMBOL(media_mem_acquire_iova);

hi_void media_mem_release_iova(const struct media_mem_file *mfile, hi_ulong len,
	const hi_void *u_virt_addr, const hi_void *k_virt_addr)
{
	(void)mfile;
	(void)len;
	(void)u_virt_addr;
	(void)k_virt_addr;
}
EXPORT_SYMBOL(media_mem_release_iova);

hi_s32 media_mem_region_check(const struct media_mem_file *mfile, hi_u64 iova_addr, hi_u64 len)
{
	struct media_compat_alloc *entry;
	hi_s32 ret = HI_FAILURE;

	(void)mfile;
	mutex_lock(&g_media_alloc_lock);
	entry = media_compat_find_by_phys(iova_addr);
	if ((entry != NULL) && (len <= entry->size))
		ret = HI_SUCCESS;
	mutex_unlock(&g_media_alloc_lock);

	return ret;
}
EXPORT_SYMBOL(media_mem_region_check);

hi_void media_mem_file_uninit(struct media_mem_file *mfile)
{
	if (mfile == NULL)
		return;

	if (mfile->mm != NULL) {
		osal_mmput(mfile->mm);
		mfile->mm = NULL;
	}
}
EXPORT_SYMBOL(media_mem_file_uninit);

hi_s32 media_kernel_mem_malloc(hi_ulong len, hi_u32 flags, hi_void **k_virt_addr)
{
	hi_u64 iova_addr = 0;

	(void)flags;
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
	hi_s32 ret = -ENOENT;

	(void)mfile;
	(void)len;
	if ((k_virt_addr == NULL) || (iova_addr == NULL))
		return -EINVAL;

	mutex_lock(&g_media_alloc_lock);
	entry = media_compat_find_by_virt(k_virt_addr);
	if (entry != NULL) {
		*iova_addr = entry->phys_addr;
		ret = HI_SUCCESS;
	}
	mutex_unlock(&g_media_alloc_lock);

	return ret;
}
EXPORT_SYMBOL(media_kernel_mem_map);

hi_s32 media_kernel_mem_unmap(struct media_mem_file *mfile, hi_ulong len, hi_u64 iova_addr)
{
	(void)mfile;
	(void)len;
	(void)iova_addr;
	return HI_SUCCESS;
}
EXPORT_SYMBOL(media_kernel_mem_unmap);

hi_s32 media_protect_mem_malloc_(struct media_mem_file *mfile, hi_ulong len, hi_u64 *iova_addr,
	hi_void **k_virt_addr, hi_u32 device_id, const hi_char *function)
{
	(void)mfile;
	(void)device_id;
	(void)function;
	return media_compat_alloc_contig(len, iova_addr, k_virt_addr);
}
EXPORT_SYMBOL(media_protect_mem_malloc_);

hi_void media_protect_mem_free_(struct media_mem_file *mfile, hi_ulong len, hi_u64 iova_addr,
	hi_void *k_virt_addr, const hi_char *function)
{
	(void)mfile;
	(void)len;
	(void)function;
	cmpi_mmz_free(iova_addr, k_virt_addr);
}
EXPORT_SYMBOL(media_protect_mem_free_);

hi_void media_protect_mem_release_iova(struct media_mem_file *mfile)
{
	(void)mfile;
}
EXPORT_SYMBOL(media_protect_mem_release_iova);

static int __init drv_base_init(void)
{
	hi_s32 i;

	mutex_lock(&g_cmpi_lock);
	for (i = 0; i < MAX_MPP_MODULES; i++) {
		cmpi_reset_module_slot(&g_cmpi_modules[i]);
		g_log_level[i] = HI_DVPP_DBG_DEBUG;
	}
	mutex_unlock(&g_cmpi_lock);

	return 0;
}

static void __exit drv_base_exit(void)
{
	struct media_compat_alloc *entry;
	struct media_compat_alloc *tmp;

	cmpi_exit_modules();

	mutex_lock(&g_media_alloc_lock);
	list_for_each_entry_safe(entry, tmp, &g_media_alloc_list, node)
		media_compat_free_contig_locked(entry);
	mutex_unlock(&g_media_alloc_lock);
}

module_init(drv_base_init);
module_exit(drv_base_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Reconstructed media base compatibility exports");
