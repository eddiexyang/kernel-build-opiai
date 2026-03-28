// SPDX-License-Identifier: GPL-2.0
#include <linux/cpumask.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/smp.h>
#include <linux/string.h>

#include "HiDvppSys.h"
#include "securec.h"
#include "sys_drv.h"
#include "sys_ext.h"

#define SYS_DEVICE_MINOR          8U
#define SYS_SCALE_BUF_LEN         31488U
#define SYS_SCALE_ALLOC_FLAGS     4352U
#define SYS_BIND_SLOT_NUM         256U
#define SYS_MEM_CTX_NUM           70U
#define SYS_VF_BITMAP_NUM         9U
#define SYS_BOOTDOT_BLOCK_ID      22U
#define SYS_PROC_TITLE_WIDTH      128U
#define SYS_PROC_TITLE_OFFSET     40U

#define SYS_RUNTIME_READY         0
#define SYS_RUNTIME_BLOCKED       1
#define SYS_RUNTIME_IDLE          2

struct sys_mem_ctx {
	hi_u32 dev_cnt;
	hi_u32 chn_cnt;
	hi_void **mmz_names;
};

struct sys_file_data {
	struct osal_list_head bind_list;
};

struct sys_sender_ctx {
	struct osal_list_head bind_list;
	hi_u32 bind_num;
};

struct sys_sender_entry {
	hi_char mod_name[MAX_MOD_NAME];
	hi_mod_id mod_id;
	hi_u32 max_dev_cnt;
	hi_u32 max_chn_cnt;
	hi_s32 (*give_bind_call_back)(hi_s32 dev_id, hi_s32 chn_id, hi_mpp_bind_dest *bind_send);
	struct sys_sender_ctx *ctxs;
};

struct sys_receiver_ctx {
	hi_bool bound;
	hi_mpp_chn src;
};

struct sys_receiver_entry {
	hi_mod_id mod_id;
	hi_u32 max_dev_cnt;
	hi_u32 max_chn_cnt;
	hi_s32 (*call_back)(hi_s32 dev_id, hi_s32 chn_id, hi_bool block,
		mpp_data_type data_type, hi_void *pv_data);
	hi_s32 (*reset_call_back)(hi_s32 dev_id, hi_s32 chn_id, hi_void *pv_data);
	hi_bool support_delay_data;
	struct sys_receiver_ctx *ctxs;
};

struct sys_bind_node {
	struct osal_list_head sender_node;
	struct osal_list_head file_node;
	hi_mpp_chn src;
	hi_mpp_chn dest;
};

typedef hi_s32 (*sys_drval_devinfo_fn)(hi_u32 device_id, hi_void *info);
typedef hi_s32 (*sys_drval_pg_info_fn)(hi_void *buf);
typedef hi_s32 (*sys_drval_pg_support_fn)(hi_void);

typedef struct {
	hi_mod_id mod_id;
	sys_scale_addr scale_addr;
	sys_scale_info scale_info;
	struct sys_sender_entry *senders[SYS_BIND_SLOT_NUM];
	struct sys_receiver_entry *receivers[SYS_BIND_SLOT_NUM];
	struct sys_mem_ctx mem_ctx[SYS_MEM_CTX_NUM];
} sys_unused_anchor_t;

static hi_s32 g_runtime_state = SYS_RUNTIME_IDLE;

hi_s32 g_sys_state = SYS_RUNTIME_IDLE;
dvpp_set_csc_param g_dvpp_set_csc_param;
dvpp_get_csc_param g_dvpp_get_csc_param;
fn_sys_get_limited_core_num *g_dvpp_jpege_get_limted_core_num;
int32_t g_is_depend_sys = 1;
hi_u32 g_device_num = 1;
hi_char g_pm_mpp_helper[PM_EVENT_HELPER_LEN];

EXPORT_SYMBOL(g_dvpp_jpege_get_limted_core_num);
EXPORT_SYMBOL(g_is_depend_sys);
EXPORT_SYMBOL(g_device_num);

static hi_u32 g_vdec_ip_num_per_device = 12;
static hi_u32 g_jpegd_ip_num_per_device = 16;
static hi_u32 g_vdec_num = 12;
static hi_u32 g_jpegd_num = 16;
static hi_u32 g_die_num = 1;
static sys_chip_type g_sys_chip_type = SYS_CHIP_MINI_V2;
static sys_platform_type g_sys_platform_type = SYS_PLAT_ASIC;
static hi_s32 g_sys_time_zone;
static hi_u64 g_global_pts_base;
static hi_u64 g_local_pts_base;
static hi_u64 g_global_pts_last;
static hi_u64 g_local_pts_last;
static hi_bool g_bootdot_init_ok;

static osal_spinlock_t g_sys_spin_lock;
static osal_spinlock_t g_sys_func_spin_lock;
static osal_spinlock_t g_sys_bind_lock;
static osal_semaphore_t g_sys_sem;
static osal_semaphore_t g_sys_ctx;
static osal_semaphore_t g_sys_dev_sem;
static osal_atomic_t g_sys_busy_cnt;
static osal_atomic_t g_sys_open_cnt;

static osal_dev_t *g_sys_device;
static osal_proc_entry_t *g_sys_proc;
static hi_mpp_sys_config g_sys_config;
static hi_bool g_conf;
static hi_gps_info g_gps_info;
static core_usable_info g_core_usable_info[DVPP_IP_TYPE_BUTT];
static dvpp_vf_bitmap_info g_dvpp_vf_bitmap[SYS_VF_BITMAP_NUM];
static cpumask_t g_irq_cpumask[DVPP_IP_TYPE_BUTT][IRQ_TYPE_BUTT][DVPP_MAX_IP_NUM_PER_DEVICE];
static struct sys_mem_ctx g_mem_ctx[SYS_MEM_CTX_NUM];
static struct sys_sender_entry *g_sender_tbl[SYS_BIND_SLOT_NUM];
static struct sys_receiver_entry *g_receiver_tbl[SYS_BIND_SLOT_NUM];
static fn_sys_proc_show *g_sys_dvpp_proc_show_func;
static fn_sys_dump_info *g_sys_dvpp_dump_info_func;
static sys_scale_info g_scale_info;
static sys_scale_addr g_scale_coef_addr;
static hi_raw_frame_compress_param g_raw_frame_compress;
static struct completion pg_info_event;

static sys_drval_devinfo_fn g_drval_devdrv_get_devinfo;
static sys_drval_pg_info_fn g_drval_get_pg_info;
static sys_drval_pg_support_fn g_drval_check_pg_info_support;
static hi_s32 (*g_svm_check_mem_attribute)(pid_t devpid, hi_u64 va, hi_u64 size, hi_u32 attr);

static hi_s32 sys_do_init(hi_void *arg);
static hi_void sys_do_exit(hi_void);
static hi_void sys_notify(mod_notice_id notice_id);
static hi_void sys_query_state(mod_state *state);
static hi_u32 sys_get_ver_magic(hi_void);
static hi_s32 sys_open(hi_void *private_data);
static hi_s32 sys_close(hi_void *private_data);
static hi_slong sys_ioctl(hi_u32 cmd, hi_ulong arg, hi_void *private_data);
static hi_slong sys_compat_ioctl(hi_u32 cmd, hi_ulong arg, hi_void *private_data);
static hi_s32 sys_freeze(const osal_dev_t *dev);
static hi_s32 sys_restore(const osal_dev_t *dev);
hi_u64 sys_get_time_stamp(hi_void);
hi_u64 sys_get_local_cur_pts(hi_void);
hi_void sys_sync_time_stamp(hi_u64 base, hi_bool init);
hi_u32 sys_drv_get_chip_version(hi_void);
hi_s32 sys_drv_get_stride(hi_u32 width, hi_u32 *stride);
hi_s32 sys_drv_drv_ioctrl(hi_mpp_chn *mpp_chn, hi_u32 func_id, hi_void *io_args);
hi_s32 sys_bind_register_sender(bind_sender_info *info);
hi_s32 sys_bind_unregister_sender(hi_mod_id mod_id);
hi_s32 sys_bind_register_receiver(bind_receiver_info *info);
hi_s32 sys_bind_unregister_receiver(hi_mod_id mod_id);
hi_s32 sys_bind_send_data(hi_mod_id mod_id, hi_s32 dev_id, hi_s32 chn_id, hi_u32 flag,
	mpp_data_type data_type, hi_void *pv_data);
hi_s32 sys_bind_reset_data(hi_mod_id mod_id, hi_s32 dev_id, hi_s32 chn_id, hi_void *pv_data);
hi_s32 sys_bind_chn(hi_mpp_chn *src_chn, hi_mpp_chn *dest_chn, hi_void *file_private);
hi_s32 sys_unbind(hi_mpp_chn *src_chn, hi_mpp_chn *dest_chn);
hi_s32 sys_get_bind_by_src(hi_mpp_chn *src_chn, hi_mpp_bind_dest *bind_dest);
hi_s32 sys_get_bind_num_by_src(hi_mpp_chn *src_chn, hi_u32 *bind_num);
hi_s32 sys_get_bind_by_dest(hi_mpp_chn *dest_chn, hi_mpp_chn *src_chn);
hi_s32 sys_get_mmz_name(hi_mpp_chn *chn, hi_void **pp_mmz_name);
hi_s32 sys_get_mem_ddr_name(const hi_mpp_chn *chn, mpp_sys_ddr_name *mem_ddr);
hi_u64 sys_get_sched_clock(hi_void);
hi_s32 sys_get_time_zone(hi_s32 *time_zone);
hi_s32 sys_get_now_string(hi_u8 *now_time);
hi_gps_info *sys_get_gps_info(hi_void);
hi_void sys_drv_get_cmp_cfg(hi_void *video_frame, hi_void *y_cmp_cfg, hi_void *c_cmp_cfg, hi_void *cmp_mode_ex_param);
hi_void sys_drv_get_vb_cfg(hi_void *info, hi_void *config);
hi_void sys_drv_get_vdec_buffer_cfg(hi_payload_type type, hi_void *info, hi_void *config);
hi_void sys_drv_get_cmp_bayer_cfg(hi_void *sys_cmp_in, hi_void *bayer_cmp_cfg, hi_void *bayer_dcmp_cfg);
hi_void sys_drv_get_cmp_3dnr_cfg(hi_void *sys_cmp_in, hi_void *y_cmp_cfg, hi_void *c_cmp_cfg);
hi_void sys_drv_get_dcmp_muv1_cfg(hi_void *sys_cmp_in, hi_void *cmp_out);
hi_void sys_drv_get_cmp_tde_cfg(hi_void *sys_cmp_in, hi_void *cmp_out);
hi_void sys_drv_get_cmp_seg_cfg(const hi_void *sys_cmp_in, hi_void *y_cmp_cfg, hi_void *c_cmp_cfg);
sys_platform_type sys_drv_get_platform_type(hi_void);
sys_chip_type sys_drv_get_chip_type(hi_void);
hi_u32 sys_get_ip_num_per_device(DVPP_IP_TYPE ip_type);
hi_u32 sys_get_ip_num(DVPP_IP_TYPE ip_type);
dvpp_set_csc_param *sys_set_csc_param_callback(hi_void);
dvpp_get_csc_param *sys_get_csc_param_callback(hi_void);
hi_s32 sys_get_cpu_info(hi_u32 device_id, hi_u32 *cpu_core_num, hi_u32 *cpu_bitmap);
hi_s32 sys_get_cpu_mask(hi_u32 device_id, hi_void *mask, CPU_TYPE get_cpu_type);
hi_s32 sys_get_vf_or_vfg_aicpu_bitmap(hi_u32 device_id, hi_u32 vf_id, hi_u32 *aicpu_bitmap, hi_u32 is_vfg);
hi_s32 sys_get_vf_or_vfg_mask(hi_u32 device_id, hi_u32 vf_id, hi_void *mask, hi_u32 is_vfg);
hi_s32 sys_set_vf_and_vfg_aicpu_bitmap(hi_u32 device_id, hi_u32 vf_id,
	hi_u32 vf_aicpu_bitmap, hi_u32 vfg_aicpu_bitmap);
hi_u32 sys_get_irq_aicpu_id_by_index(hi_u32 device_id, hi_u32 vf_id, hi_u32 idx);
hi_s32 sys_set_aicpu_bind_index(hi_u32 device_id, hi_u32 vf_id);
hi_u32 sys_get_cpu_id_by_index(hi_u32 device_id, hi_u32 idx, CPU_TYPE get_cpu_type);
hi_s32 sys_bind_irq_to_cpu(dvpp_core_info core_info, hi_u32 cpu_id);
hi_s32 sys_bind_irq_to_cpu_mask(hi_u32 irq, const hi_void *mask);
hi_s32 sys_unbind_irq_to_cpu(hi_u32 irq);
hi_void sys_bootdot_init_block(hi_u64 exception_id, PROCESS_TYPE process_type, hi_u8 core_id, hi_u8 status);
hi_void sys_bootdot_set_block_status(PROCESS_TYPE process_type, hi_u8 core_id, hi_u8 status);
hi_s32 sys_get_core_usable_info(DVPP_IP_TYPE core_type, core_usable_info *usable_info);
hi_void sys_export_func_spin_lock(hi_void);
hi_void sys_export_func_spin_unlock(hi_void);
hi_s32 sys_print_proc_title(osal_proc_entry_t *entry, const hi_char *format, ...)
	__attribute__((format(printf, 2, 3)));
hi_u32 sys_get_scale_coef(hi_mod_id mod_id, hi_void *scale_coef_opt_ptr, hi_void *pv_rate, hi_void *pv_cfg);
hi_void sys_get_scale_coef_kva(hi_void **scale_coef_addr, hi_u64 *coef_len);
hi_s32 sys_set_device_num(hi_u32 device_num);
hi_u32 sys_get_die_num_per_device(hi_void);
hi_s32 sys_get_aicpu_bitmap(hi_u32 ext_phy_dev_id, hi_u32 *cpu_bitmap);
hi_void sys_regist_dvpp_proc_show(fn_sys_proc_show *func);
hi_void sys_unregist_dvpp_proc_show(hi_void);
hi_void sys_regist_dvpp_dump_info(fn_sys_dump_info *func);
hi_void sys_unregist_dvpp_dump_info(hi_void);

static osal_fileops_t g_sys_fops = {
	.open = sys_open,
	.release = sys_close,
	.unlocked_ioctl = sys_ioctl,
#if defined(CONFIG_COMPAT) || defined(AOS_LLVM_BUILD)
	.compat_ioctl = sys_compat_ioctl,
#endif
	.module = THIS_MODULE,
};

static osal_pmops_t g_sys_pmops = {
	.pm_freeze = sys_freeze,
	.pm_restore = sys_restore,
};

static sys_export_func g_sys_export_funcs = {
	.pfn_sys_get_time_stamp = sys_get_time_stamp,
	.pfn_sys_get_local_time_stamp = sys_get_local_cur_pts,
	.pfn_sys_sync_time_stamp = sys_sync_time_stamp,
	.pfn_sys_get_chip_version = sys_drv_get_chip_version,
	.pfn_sys_get_stride = sys_drv_get_stride,
	.pfn_sys_drv_ioctrl = sys_drv_drv_ioctrl,
	.pfn_sys_register_sender = sys_bind_register_sender,
	.pfn_sys_unregister_sender = sys_bind_unregister_sender,
	.pfn_sys_register_receiver = sys_bind_register_receiver,
	.pfn_sys_unregister_receiver = sys_bind_unregister_receiver,
	.pfn_sys_send_data = sys_bind_send_data,
	.pfn_sys_reset_data = sys_bind_reset_data,
	.pfn_get_bind_by_src = sys_get_bind_by_src,
	.pfn_get_bind_num_by_src = sys_get_bind_num_by_src,
	.pfn_get_bind_by_dest = sys_get_bind_by_dest,
	.pfn_get_mmz_name = sys_get_mmz_name,
	.pfn_get_mem_ddr = sys_get_mem_ddr_name,
	.pfn_sys_get_sched_clock = sys_get_sched_clock,
	.pfn_sys_get_time_zone = sys_get_time_zone,
	.pfn_sys_get_now_string = sys_get_now_string,
	.pfn_sys_get_gps_info = sys_get_gps_info,
	.pfn_sys_get_cmp_cfg = sys_drv_get_cmp_cfg,
	.pfn_sys_get_vb_cfg = sys_drv_get_vb_cfg,
	.pfn_sys_get_vdec_buffer_cfg = sys_drv_get_vdec_buffer_cfg,
	.pfn_sys_get_cmp_bayer_cfg = sys_drv_get_cmp_bayer_cfg,
	.pfn_sys_get_cmp_3dnr_cfg = sys_drv_get_cmp_3dnr_cfg,
	.pfn_sys_get_dcmp_muv1_cfg = sys_drv_get_dcmp_muv1_cfg,
	.pfn_sys_get_cmp_tde_cfg = sys_drv_get_cmp_tde_cfg,
	.pfn_sys_get_cmp_seg_cfg = sys_drv_get_cmp_seg_cfg,
	.pfn_sys_get_platform_type = sys_drv_get_platform_type,
	.pfn_sys_get_chip_type = sys_drv_get_chip_type,
	.pfn_sys_get_ip_num_per_device = sys_get_ip_num_per_device,
	.pfn_sys_get_ip_num = sys_get_ip_num,
	.pfn_sys_set_csc_param_callback = sys_set_csc_param_callback,
	.pfn_sys_get_csc_param_callback = sys_get_csc_param_callback,
	.pfn_sys_get_cpu_info = sys_get_cpu_info,
	.pfn_sys_get_cpu_mask = sys_get_cpu_mask,
	.pfn_sys_get_vf_or_vfg_aicpu_bitmap = sys_get_vf_or_vfg_aicpu_bitmap,
	.pfn_sys_get_vf_or_vfg_mask = sys_get_vf_or_vfg_mask,
	.pfn_sys_set_vf_and_vfg_aicpu_bitmap = sys_set_vf_and_vfg_aicpu_bitmap,
	.pfn_sys_get_irq_aicpu_id_by_index = sys_get_irq_aicpu_id_by_index,
	.pfn_sys_set_aicpu_bind_index = sys_set_aicpu_bind_index,
	.pfn_sys_get_cpu_id_by_index = sys_get_cpu_id_by_index,
	.pfn_sys_bind_irq_to_cpu = sys_bind_irq_to_cpu,
	.pfn_sys_bind_irq_to_cpu_mask = sys_bind_irq_to_cpu_mask,
	.pfn_sys_unbind_irq_to_cpu = sys_unbind_irq_to_cpu,
	.pfn_sys_bootdot_init_block = sys_bootdot_init_block,
	.pfn_sys_bootdot_set_block_status = sys_bootdot_set_block_status,
	.pfn_sys_get_core_usable_info = sys_get_core_usable_info,
	.pfn_sys_export_func_spin_lock = sys_export_func_spin_lock,
	.pfn_sys_export_func_spin_unlock = sys_export_func_spin_unlock,
	.pfn_sys_print_proc_title = sys_print_proc_title,
	.pfn_sys_get_scale_coef = sys_get_scale_coef,
	.pfn_sys_get_scale_coef_kva = sys_get_scale_coef_kva,
	.pfn_sys_set_device_num = sys_set_device_num,
	.pfn_sys_get_die_num_per_device = sys_get_die_num_per_device,
	.pfn_sys_get_aicpu_bitmap = sys_get_aicpu_bitmap,
	.pfn_sys_regist_dvpp_proc_show = sys_regist_dvpp_proc_show,
	.pfn_sys_unregist_dvpp_proc_show = sys_unregist_dvpp_proc_show,
	.pfn_sys_regist_dvpp_dump_info = sys_regist_dvpp_dump_info,
	.pfn_sys_unregist_dvpp_dump_info = sys_unregist_dvpp_dump_info,
};

static umap_module g_sys_module = {
	.mod_name = "sys",
	.mod_id = HI_ID_SYS,
	.pfn_init = sys_do_init,
	.pfn_exit = sys_do_exit,
	.pfn_query_state = sys_query_state,
	.pfn_notify = sys_notify,
	.pfn_ver_checker = sys_get_ver_magic,
	.export_funcs = &g_sys_export_funcs,
};

static hi_bool sys_chip_is_mini_v2_family(hi_void)
{
	return (g_sys_chip_type == SYS_CHIP_MINI_V2 || g_sys_chip_type == SYS_CHIP_MINI_V2_MDC);
}

static hi_u32 sys_online_cpu_count(hi_void)
{
	hi_u32 cpu_cnt = (hi_u32)num_online_cpus();

	if (cpu_cnt == 0U)
		cpu_cnt = 1U;
	return cpu_cnt;
}

static hi_u32 sys_default_cpu_bitmap(hi_void)
{
	hi_u32 cpu_cnt = osal_min(sys_online_cpu_count(), 32U);

	if (cpu_cnt >= 32U)
		return 0xFFFFFFFFU;
	return (1U << cpu_cnt) - 1U;
}

static hi_s32 sys_copy_to_arg(hi_void *dst, const hi_void *src, size_t len)
{
	if (dst == HI_NULL || src == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	if (memcpy_s(dst, len, src, len) != EOK)
		return HI_ERR_SYS_ERR;
	return HI_SUCCESS;
}

static hi_s32 sys_check_bind_mod_id(hi_mod_id mod_id)
{
	if ((hi_u32)mod_id >= SYS_BIND_SLOT_NUM)
		return HI_ERR_SYS_ILLEGAL_PARAM;
	return HI_SUCCESS;
}

static hi_s32 sys_get_sender_index(hi_mod_id mod_id, hi_s32 dev_id, hi_s32 chn_id, hi_u32 *idx)
{
	struct sys_sender_entry *sender;

	if (idx == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	if (sys_check_bind_mod_id(mod_id) != HI_SUCCESS)
		return HI_ERR_SYS_ILLEGAL_PARAM;

	BIND_ADJUST_SRC_DEVID(mod_id, dev_id);
	BIND_ADJUST_SRC_CHNID(mod_id, chn_id);

	sender = g_sender_tbl[mod_id];
	if (sender == HI_NULL)
		return HI_ERR_SYS_UNEXIST;
	if (dev_id < 0 || chn_id < 0)
		return HI_ERR_SYS_ILLEGAL_PARAM;
	if ((hi_u32)dev_id >= sender->max_dev_cnt || (hi_u32)chn_id >= sender->max_chn_cnt)
		return HI_ERR_SYS_ILLEGAL_PARAM;

	*idx = (hi_u32)dev_id * sender->max_chn_cnt + (hi_u32)chn_id;
	return HI_SUCCESS;
}

static hi_s32 sys_get_receiver_index(hi_mod_id mod_id, hi_s32 dev_id, hi_s32 chn_id, hi_u32 *idx)
{
	struct sys_receiver_entry *receiver;

	if (idx == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	if (sys_check_bind_mod_id(mod_id) != HI_SUCCESS)
		return HI_ERR_SYS_ILLEGAL_PARAM;

	BIND_ADJUST_DEST_DEVID(mod_id, dev_id);

	receiver = g_receiver_tbl[mod_id];
	if (receiver == HI_NULL)
		return HI_ERR_SYS_UNEXIST;
	if (dev_id < 0 || chn_id < 0)
		return HI_ERR_SYS_ILLEGAL_PARAM;
	if ((hi_u32)dev_id >= receiver->max_dev_cnt || (hi_u32)chn_id >= receiver->max_chn_cnt)
		return HI_ERR_SYS_ILLEGAL_PARAM;

	*idx = (hi_u32)dev_id * receiver->max_chn_cnt + (hi_u32)chn_id;
	return HI_SUCCESS;
}

static hi_void sys_notify_sender_bind_change(const hi_mpp_chn *src)
{
	struct sys_sender_entry *sender;
	hi_u32 idx;
	hi_mpp_bind_dest bind_dest;

	if (src == HI_NULL)
		return;
	if (sys_get_sender_index(src->mod_id, src->dev_id, src->chn_id, &idx) != HI_SUCCESS)
		return;

	sender = g_sender_tbl[src->mod_id];
	if (sender == HI_NULL || sender->give_bind_call_back == HI_NULL)
		return;

	memset(&bind_dest, 0, sizeof(bind_dest));
	(void)sys_get_bind_by_src((hi_mpp_chn *)src, &bind_dest);
	(void)sender->give_bind_call_back(src->dev_id, src->chn_id, &bind_dest);
}

static hi_s32 sys_bind_mod_init(hi_void)
{
	hi_s32 ret;

	ret = osal_spin_lock_init(&g_sys_bind_lock);
	if (ret != HI_SUCCESS)
		return ret;
	ret = osal_sema_init(&g_sys_dev_sem, 1);
	if (ret != HI_SUCCESS) {
		osal_spin_lock_destory(&g_sys_bind_lock);
		return ret;
	}
	return HI_SUCCESS;
}

static hi_void sys_bind_mod_exit(hi_void)
{
	osal_sema_destory(&g_sys_dev_sem);
	osal_spin_lock_destory(&g_sys_bind_lock);
}

static hi_void sys_bind_init(hi_void)
{
	memset(g_sender_tbl, 0, sizeof(g_sender_tbl));
	memset(g_receiver_tbl, 0, sizeof(g_receiver_tbl));
}

static hi_void sys_release_sender_nodes_locked(struct sys_sender_entry *sender)
{
	hi_u32 i;
	struct osal_list_head *pos = HI_NULL;
	struct osal_list_head *tmp = HI_NULL;

	if (sender == HI_NULL || sender->ctxs == HI_NULL)
		return;

	for (i = 0; i < sender->max_dev_cnt * sender->max_chn_cnt; ++i) {
		struct sys_sender_ctx *ctx = &sender->ctxs[i];

		osal_list_for_each_safe(pos, tmp, &ctx->bind_list) {
			struct sys_bind_node *node = osal_list_entry(pos, struct sys_bind_node, sender_node);
			hi_u32 ridx;
			struct sys_receiver_entry *receiver = HI_NULL;

			if (sys_get_receiver_index(node->dest.mod_id, node->dest.dev_id, node->dest.chn_id, &ridx) == HI_SUCCESS)
				receiver = g_receiver_tbl[node->dest.mod_id];
			if (receiver != HI_NULL && receiver->ctxs != HI_NULL) {
				receiver->ctxs[ridx].bound = HI_FALSE;
				memset(&receiver->ctxs[ridx].src, 0, sizeof(receiver->ctxs[ridx].src));
			}
			osal_list_del(&node->sender_node);
			osal_list_del(&node->file_node);
			osal_kfree(node);
		}
		ctx->bind_num = 0;
		OSAL_INIT_LIST_HEAD(&ctx->bind_list);
	}
}

static hi_void sys_bind_exit(hi_void)
{
	hi_u32 i;

	for (i = 0; i < SYS_BIND_SLOT_NUM; ++i) {
		if (g_sender_tbl[i] != HI_NULL) {
			sys_release_sender_nodes_locked(g_sender_tbl[i]);
			osal_kfree(g_sender_tbl[i]->ctxs);
			osal_kfree(g_sender_tbl[i]);
			g_sender_tbl[i] = HI_NULL;
		}
		if (g_receiver_tbl[i] != HI_NULL) {
			osal_kfree(g_receiver_tbl[i]->ctxs);
			osal_kfree(g_receiver_tbl[i]);
			g_receiver_tbl[i] = HI_NULL;
		}
	}
}

hi_s32 sys_bind_register_sender(bind_sender_info *info)
{
	struct sys_sender_entry *sender = HI_NULL;
	hi_u32 i;
	hi_u32 ctx_num;
	hi_char *name = HI_NULL;

	if (info == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	if (sys_check_bind_mod_id(info->mod_id) != HI_SUCCESS)
		return HI_ERR_SYS_ILLEGAL_PARAM;
	if (info->max_dev_cnt == 0U || info->max_chn_cnt == 0U)
		return HI_ERR_SYS_ILLEGAL_PARAM;
	if (g_sender_tbl[info->mod_id] != HI_NULL)
		return HI_ERR_SYS_EXIST;

	sender = osal_kmalloc(sizeof(*sender), osal_gfp_kernel);
	if (sender == HI_NULL)
		return HI_ERR_SYS_NOMEM;
	memset(sender, 0, sizeof(*sender));

	ctx_num = info->max_dev_cnt * info->max_chn_cnt;
	sender->ctxs = osal_kmalloc(sizeof(*sender->ctxs) * ctx_num, osal_gfp_kernel);
	if (sender->ctxs == HI_NULL) {
		osal_kfree(sender);
		return HI_ERR_SYS_NOMEM;
	}
	memset(sender->ctxs, 0, sizeof(*sender->ctxs) * ctx_num);

	name = cmpi_get_module_name(info->mod_id);
	if (name != HI_NULL)
		(void)strncpy_s(sender->mod_name, sizeof(sender->mod_name), name, sizeof(sender->mod_name) - 1U);
	else
		(void)strncpy_s(sender->mod_name, sizeof(sender->mod_name), "unknow mod", sizeof(sender->mod_name) - 1U);

	sender->mod_id = info->mod_id;
	sender->max_dev_cnt = info->max_dev_cnt;
	sender->max_chn_cnt = info->max_chn_cnt;
	sender->give_bind_call_back = info->give_bind_call_back;
	for (i = 0; i < ctx_num; ++i)
		OSAL_INIT_LIST_HEAD(&sender->ctxs[i].bind_list);

	osal_spin_lock(&g_sys_bind_lock);
	g_sender_tbl[info->mod_id] = sender;
	osal_spin_unlock(&g_sys_bind_lock);
	return HI_SUCCESS;
}

hi_s32 sys_bind_unregister_sender(hi_mod_id mod_id)
{
	struct sys_sender_entry *sender;

	if (sys_check_bind_mod_id(mod_id) != HI_SUCCESS)
		return HI_ERR_SYS_ILLEGAL_PARAM;

	osal_spin_lock(&g_sys_bind_lock);
	sender = g_sender_tbl[mod_id];
	if (sender == HI_NULL) {
		osal_spin_unlock(&g_sys_bind_lock);
		return HI_ERR_SYS_UNEXIST;
	}
	sys_release_sender_nodes_locked(sender);
	g_sender_tbl[mod_id] = HI_NULL;
	osal_spin_unlock(&g_sys_bind_lock);

	osal_kfree(sender->ctxs);
	osal_kfree(sender);
	return HI_SUCCESS;
}

hi_s32 sys_bind_register_receiver(bind_receiver_info *info)
{
	struct sys_receiver_entry *receiver;
	hi_u32 ctx_num;

	if (info == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	if (sys_check_bind_mod_id(info->mod_id) != HI_SUCCESS)
		return HI_ERR_SYS_ILLEGAL_PARAM;
	if (info->max_dev_cnt == 0U || info->max_chn_cnt == 0U)
		return HI_ERR_SYS_ILLEGAL_PARAM;
	if (g_receiver_tbl[info->mod_id] != HI_NULL)
		return HI_ERR_SYS_EXIST;

	receiver = osal_kmalloc(sizeof(*receiver), osal_gfp_kernel);
	if (receiver == HI_NULL)
		return HI_ERR_SYS_NOMEM;
	memset(receiver, 0, sizeof(*receiver));

	ctx_num = info->max_dev_cnt * info->max_chn_cnt;
	receiver->ctxs = osal_kmalloc(sizeof(*receiver->ctxs) * ctx_num, osal_gfp_kernel);
	if (receiver->ctxs == HI_NULL) {
		osal_kfree(receiver);
		return HI_ERR_SYS_NOMEM;
	}
	memset(receiver->ctxs, 0, sizeof(*receiver->ctxs) * ctx_num);

	receiver->mod_id = info->mod_id;
	receiver->max_dev_cnt = info->max_dev_cnt;
	receiver->max_chn_cnt = info->max_chn_cnt;
	receiver->call_back = info->call_back;
	receiver->reset_call_back = info->reset_call_back;
	receiver->support_delay_data = info->support_delay_data;

	osal_spin_lock(&g_sys_bind_lock);
	g_receiver_tbl[info->mod_id] = receiver;
	osal_spin_unlock(&g_sys_bind_lock);
	return HI_SUCCESS;
}

hi_s32 sys_bind_unregister_receiver(hi_mod_id mod_id)
{
	struct sys_receiver_entry *receiver;
	hi_u32 i;
	hi_u32 j;

	if (sys_check_bind_mod_id(mod_id) != HI_SUCCESS)
		return HI_ERR_SYS_ILLEGAL_PARAM;

	osal_spin_lock(&g_sys_bind_lock);
	receiver = g_receiver_tbl[mod_id];
	if (receiver == HI_NULL) {
		osal_spin_unlock(&g_sys_bind_lock);
		return HI_ERR_SYS_UNEXIST;
	}

	for (i = 0; i < SYS_BIND_SLOT_NUM; ++i) {
		struct sys_sender_entry *sender = g_sender_tbl[i];

		if (sender == HI_NULL || sender->ctxs == HI_NULL)
			continue;
		for (j = 0; j < sender->max_dev_cnt * sender->max_chn_cnt; ++j) {
			struct osal_list_head *pos = HI_NULL;
			struct osal_list_head *tmp = HI_NULL;

			osal_list_for_each_safe(pos, tmp, &sender->ctxs[j].bind_list) {
				struct sys_bind_node *node = osal_list_entry(pos, struct sys_bind_node, sender_node);

				if (node->dest.mod_id != mod_id)
					continue;
				osal_list_del(&node->sender_node);
				osal_list_del(&node->file_node);
				if (sender->ctxs[j].bind_num > 0U)
					sender->ctxs[j].bind_num--;
				osal_kfree(node);
			}
		}
	}

	g_receiver_tbl[mod_id] = HI_NULL;
	osal_spin_unlock(&g_sys_bind_lock);

	osal_kfree(receiver->ctxs);
	osal_kfree(receiver);
	return HI_SUCCESS;
}

static hi_s32 sys_get_bind_by_dest_inner(const hi_mpp_chn *dest_chn, hi_mpp_chn *src_chn, hi_bool warn_if_unbound)
{
	struct sys_receiver_entry *receiver;
	hi_u32 idx;
	hi_s32 ret;

	if (dest_chn == HI_NULL || src_chn == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;

	ret = sys_get_receiver_index(dest_chn->mod_id, dest_chn->dev_id, dest_chn->chn_id, &idx);
	if (ret != HI_SUCCESS)
		return ret;

	osal_spin_lock(&g_sys_bind_lock);
	receiver = g_receiver_tbl[dest_chn->mod_id];
	if (receiver == HI_NULL || receiver->ctxs == HI_NULL || receiver->ctxs[idx].bound == HI_FALSE) {
		osal_spin_unlock(&g_sys_bind_lock);
		if (warn_if_unbound)
			HI_WARN_TRACE(HI_ID_SYS, "dest(%d,%d,%d) unbound\n",
				dest_chn->mod_id, dest_chn->dev_id, dest_chn->chn_id);
		return HI_ERR_SYS_UNEXIST;
	}
	*src_chn = receiver->ctxs[idx].src;
	osal_spin_unlock(&g_sys_bind_lock);
	return HI_SUCCESS;
}

hi_s32 sys_get_bind_by_dest(hi_mpp_chn *dest_chn, hi_mpp_chn *src_chn)
{
	return sys_get_bind_by_dest_inner(dest_chn, src_chn, HI_TRUE);
}

hi_s32 sys_get_bind_num_by_src(hi_mpp_chn *src_chn, hi_u32 *bind_num)
{
	struct sys_sender_entry *sender;
	hi_u32 idx;
	hi_s32 ret;

	if (src_chn == HI_NULL || bind_num == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;

	ret = sys_get_sender_index(src_chn->mod_id, src_chn->dev_id, src_chn->chn_id, &idx);
	if (ret != HI_SUCCESS)
		return ret;

	osal_spin_lock(&g_sys_bind_lock);
	sender = g_sender_tbl[src_chn->mod_id];
	if (sender == HI_NULL || sender->ctxs == HI_NULL) {
		osal_spin_unlock(&g_sys_bind_lock);
		return HI_ERR_SYS_UNEXIST;
	}
	*bind_num = sender->ctxs[idx].bind_num;
	osal_spin_unlock(&g_sys_bind_lock);
	return HI_SUCCESS;
}

hi_s32 sys_get_bind_by_src(hi_mpp_chn *src_chn, hi_mpp_bind_dest *bind_dest)
{
	struct sys_sender_entry *sender;
	hi_u32 idx;
	hi_s32 ret;
	struct osal_list_head *pos;

	if (src_chn == HI_NULL || bind_dest == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;

	ret = sys_get_sender_index(src_chn->mod_id, src_chn->dev_id, src_chn->chn_id, &idx);
	if (ret != HI_SUCCESS)
		return ret;

	memset(bind_dest, 0, sizeof(*bind_dest));

	if (osal_down(&g_sys_dev_sem) != HI_SUCCESS)
		return HI_ERR_SYS_ERR;

	osal_spin_lock(&g_sys_bind_lock);
	sender = g_sender_tbl[src_chn->mod_id];
	if (sender != HI_NULL && sender->ctxs != HI_NULL) {
		osal_list_for_each(pos, &sender->ctxs[idx].bind_list) {
			struct sys_bind_node *node;

			if (bind_dest->num >= BIND_DEST_MAXNUM)
				break;
			node = osal_list_entry(pos, struct sys_bind_node, sender_node);
			bind_dest->mpp_chn[bind_dest->num++] = node->dest;
		}
	}
	osal_spin_unlock(&g_sys_bind_lock);
	osal_up(&g_sys_dev_sem);
	return HI_SUCCESS;
}

hi_s32 sys_bind_send_data(hi_mod_id mod_id, hi_s32 dev_id, hi_s32 chn_id, hi_u32 flag,
	mpp_data_type data_type, hi_void *pv_data)
{
	struct {
		hi_s32 (*fn)(hi_s32 dev_id, hi_s32 chn_id, hi_bool block, mpp_data_type data_type, hi_void *pv_data);
		hi_s32 dev_id;
		hi_s32 chn_id;
	} call_list[BIND_DEST_MAXNUM];
	struct sys_sender_entry *sender;
	hi_u32 idx;
	hi_u32 cnt = 0;
	struct osal_list_head *pos;
	hi_bool block = ((flag & SYS_SEND_DATA_BLOCK_MASK) != 0U);
	hi_s32 ret;

	ret = sys_get_sender_index(mod_id, dev_id, chn_id, &idx);
	if (ret != HI_SUCCESS)
		return ret;

	osal_spin_lock(&g_sys_bind_lock);
	sender = g_sender_tbl[mod_id];
	if (sender != HI_NULL && sender->ctxs != HI_NULL) {
		osal_list_for_each(pos, &sender->ctxs[idx].bind_list) {
			struct sys_bind_node *node = osal_list_entry(pos, struct sys_bind_node, sender_node);
			struct sys_receiver_entry *receiver;
			hi_u32 ridx;

			if (cnt >= BIND_DEST_MAXNUM)
				break;
			if (sys_get_receiver_index(node->dest.mod_id, node->dest.dev_id, node->dest.chn_id, &ridx) != HI_SUCCESS)
				continue;
			receiver = g_receiver_tbl[node->dest.mod_id];
			if (receiver == HI_NULL || receiver->call_back == HI_NULL)
				continue;
			call_list[cnt].fn = receiver->call_back;
			call_list[cnt].dev_id = node->dest.dev_id;
			call_list[cnt].chn_id = node->dest.chn_id;
			BIND_ADJUST_DEST_DEVID(node->dest.mod_id, call_list[cnt].dev_id);
			cnt++;
		}
	}
	osal_spin_unlock(&g_sys_bind_lock);

	for (idx = 0; idx < cnt; ++idx)
		(void)call_list[idx].fn(call_list[idx].dev_id, call_list[idx].chn_id, block, data_type, pv_data);

	return HI_SUCCESS;
}

hi_s32 sys_bind_reset_data(hi_mod_id mod_id, hi_s32 dev_id, hi_s32 chn_id, hi_void *pv_data)
{
	struct {
		hi_s32 (*fn)(hi_s32 dev_id, hi_s32 chn_id, hi_void *pv_data);
		hi_s32 dev_id;
		hi_s32 chn_id;
	} call_list[BIND_DEST_MAXNUM];
	struct sys_sender_entry *sender;
	hi_u32 idx;
	hi_u32 cnt = 0;
	struct osal_list_head *pos;
	hi_s32 ret;

	ret = sys_get_sender_index(mod_id, dev_id, chn_id, &idx);
	if (ret != HI_SUCCESS)
		return ret;

	osal_spin_lock(&g_sys_bind_lock);
	sender = g_sender_tbl[mod_id];
	if (sender != HI_NULL && sender->ctxs != HI_NULL) {
		osal_list_for_each(pos, &sender->ctxs[idx].bind_list) {
			struct sys_bind_node *node = osal_list_entry(pos, struct sys_bind_node, sender_node);
			struct sys_receiver_entry *receiver;
			hi_u32 ridx;

			if (cnt >= BIND_DEST_MAXNUM)
				break;
			if (sys_get_receiver_index(node->dest.mod_id, node->dest.dev_id, node->dest.chn_id, &ridx) != HI_SUCCESS)
				continue;
			receiver = g_receiver_tbl[node->dest.mod_id];
			if (receiver == HI_NULL || receiver->reset_call_back == HI_NULL)
				continue;
			call_list[cnt].fn = receiver->reset_call_back;
			call_list[cnt].dev_id = node->dest.dev_id;
			call_list[cnt].chn_id = node->dest.chn_id;
			BIND_ADJUST_DEST_DEVID(node->dest.mod_id, call_list[cnt].dev_id);
			cnt++;
		}
	}
	osal_spin_unlock(&g_sys_bind_lock);

	for (idx = 0; idx < cnt; ++idx)
		(void)call_list[idx].fn(call_list[idx].dev_id, call_list[idx].chn_id, pv_data);

	return HI_SUCCESS;
}

static hi_s32 sys_really_bind_locked(const hi_mpp_chn *src, const hi_mpp_chn *dest, struct sys_file_data *file_data)
{
	struct sys_sender_entry *sender;
	struct sys_receiver_entry *receiver;
	struct sys_bind_node *node;
	hi_u32 sidx;
	hi_u32 ridx;
	hi_s32 ret;

	ret = sys_get_sender_index(src->mod_id, src->dev_id, src->chn_id, &sidx);
	if (ret != HI_SUCCESS)
		return ret;
	ret = sys_get_receiver_index(dest->mod_id, dest->dev_id, dest->chn_id, &ridx);
	if (ret != HI_SUCCESS)
		return ret;

	sender = g_sender_tbl[src->mod_id];
	receiver = g_receiver_tbl[dest->mod_id];
	if (sender == HI_NULL || receiver == HI_NULL || sender->ctxs == HI_NULL || receiver->ctxs == HI_NULL)
		return HI_ERR_SYS_UNEXIST;

	if (receiver->ctxs[ridx].bound == HI_TRUE)
		return HI_ERR_SYS_EXIST;
	if (sender->ctxs[sidx].bind_num >= BIND_DEST_MAXNUM)
		return HI_ERR_SYS_EXIST;

	node = osal_kmalloc(sizeof(*node), osal_gfp_kernel);
	if (node == HI_NULL)
		return HI_ERR_SYS_NOMEM;
	memset(node, 0, sizeof(*node));
	node->src = *src;
	node->dest = *dest;
	OSAL_INIT_LIST_HEAD(&node->sender_node);
	OSAL_INIT_LIST_HEAD(&node->file_node);

	receiver->ctxs[ridx].bound = HI_TRUE;
	receiver->ctxs[ridx].src = *src;
	osal_list_add_tail(&node->sender_node, &sender->ctxs[sidx].bind_list);
	osal_list_add_tail(&node->file_node, &file_data->bind_list);
	sender->ctxs[sidx].bind_num++;
	return HI_SUCCESS;
}

hi_s32 sys_bind_chn(hi_mpp_chn *src_chn, hi_mpp_chn *dest_chn, hi_void *file_private)
{
	struct sys_file_data *file_data = file_private;
	hi_s32 ret;

	if (src_chn == HI_NULL || dest_chn == HI_NULL || file_data == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;

	ret = osal_down(&g_sys_dev_sem);
	if (ret != HI_SUCCESS)
		return HI_ERR_SYS_ERR;
	osal_spin_lock(&g_sys_bind_lock);
	ret = sys_really_bind_locked(src_chn, dest_chn, file_data);
	osal_spin_unlock(&g_sys_bind_lock);
	osal_up(&g_sys_dev_sem);

	if (ret == HI_SUCCESS)
		sys_notify_sender_bind_change(src_chn);
	return ret;
}

hi_s32 sys_unbind(hi_mpp_chn *src_chn, hi_mpp_chn *dest_chn)
{
	struct sys_sender_entry *sender;
	struct sys_receiver_entry *receiver;
	hi_u32 sidx;
	hi_u32 ridx;
	struct osal_list_head *pos = HI_NULL;
	struct osal_list_head *tmp = HI_NULL;
	hi_s32 ret;
	hi_s32 dest_dev_id;
	hi_s32 dest_chn_id;
	hi_s32 (*reset_cb)(hi_s32 dev_id, hi_s32 chn_id, hi_void *pv_data) = HI_NULL;

	if (src_chn == HI_NULL || dest_chn == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;

	ret = sys_get_sender_index(src_chn->mod_id, src_chn->dev_id, src_chn->chn_id, &sidx);
	if (ret != HI_SUCCESS)
		return ret;
	ret = sys_get_receiver_index(dest_chn->mod_id, dest_chn->dev_id, dest_chn->chn_id, &ridx);
	if (ret != HI_SUCCESS)
		return ret;

	ret = osal_down(&g_sys_dev_sem);
	if (ret != HI_SUCCESS)
		return HI_ERR_SYS_ERR;

	osal_spin_lock(&g_sys_bind_lock);
	sender = g_sender_tbl[src_chn->mod_id];
	receiver = g_receiver_tbl[dest_chn->mod_id];
	if (sender == HI_NULL || receiver == HI_NULL || sender->ctxs == HI_NULL || receiver->ctxs == HI_NULL) {
		ret = HI_ERR_SYS_UNEXIST;
		goto out_unlock;
	}

	if (receiver->ctxs[ridx].bound == HI_FALSE ||
		memcmp(&receiver->ctxs[ridx].src, src_chn, sizeof(*src_chn)) != 0) {
		ret = HI_ERR_SYS_UNEXIST;
		goto out_unlock;
	}

	osal_list_for_each_safe(pos, tmp, &sender->ctxs[sidx].bind_list) {
		struct sys_bind_node *node = osal_list_entry(pos, struct sys_bind_node, sender_node);

		if (memcmp(&node->dest, dest_chn, sizeof(*dest_chn)) != 0)
			continue;

		osal_list_del(&node->sender_node);
		osal_list_del(&node->file_node);
		osal_kfree(node);
		if (sender->ctxs[sidx].bind_num > 0U)
			sender->ctxs[sidx].bind_num--;
		break;
	}

	receiver->ctxs[ridx].bound = HI_FALSE;
	memset(&receiver->ctxs[ridx].src, 0, sizeof(receiver->ctxs[ridx].src));
	reset_cb = receiver->reset_call_back;
	dest_dev_id = dest_chn->dev_id;
	dest_chn_id = dest_chn->chn_id;
	BIND_ADJUST_DEST_DEVID(dest_chn->mod_id, dest_dev_id);
	ret = HI_SUCCESS;

out_unlock:
	osal_spin_unlock(&g_sys_bind_lock);
	osal_up(&g_sys_dev_sem);

	if (ret == HI_SUCCESS) {
		sys_notify_sender_bind_change(src_chn);
		if (src_chn->mod_id == HI_ID_VDEC && reset_cb != HI_NULL)
			(void)reset_cb(dest_dev_id, dest_chn_id, HI_NULL);
	}
	return ret;
}

static hi_void sys_destroy_bind_info(struct sys_file_data *file_data)
{
	while (file_data != HI_NULL && !osal_list_empty(&file_data->bind_list)) {
		struct sys_bind_node *node;
		hi_mpp_chn src;
		hi_mpp_chn dest;

		node = osal_list_first_entry(&file_data->bind_list, struct sys_bind_node, file_node);
		src = node->src;
		dest = node->dest;
		(void)sys_unbind(&src, &dest);
	}
}

static hi_s32 create_sys_file_data(hi_void *private_data)
{
	struct sys_file_data *file_data;
	hi_void **data = private_data;

	if (data == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;

	file_data = osal_kmalloc(sizeof(*file_data), osal_gfp_kernel);
	if (file_data == HI_NULL)
		return HI_ERR_SYS_NOMEM;
	memset(file_data, 0, sizeof(*file_data));
	OSAL_INIT_LIST_HEAD(&file_data->bind_list);
	*data = file_data;
	return HI_SUCCESS;
}

static hi_void destroy_sys_file_data(hi_void *private_data)
{
	hi_void **data = private_data;
	struct sys_file_data *file_data;

	if (data == HI_NULL || *data == HI_NULL)
		return;
	file_data = *data;
	sys_destroy_bind_info(file_data);
	osal_kfree(file_data);
	*data = HI_NULL;
}

hi_u64 sys_get_sched_clock(hi_void)
{
	return osal_sched_clock();
}

hi_u64 sys_get_local_cur_pts(hi_void)
{
	hi_u64 now = osal_div_u64(osal_sched_clock(), 1000U);

	if (now < g_local_pts_last)
		HI_WARN_TRACE(HI_ID_SYS, "local pts rollback: last=%llu now=%llu\n", g_local_pts_last, now);
	g_local_pts_last = now;
	return now;
}

hi_void sys_sync_time_stamp(hi_u64 base, hi_bool init)
{
	hi_ulong flags = 0;

	osal_spin_lock_irqsave(&g_sys_spin_lock, &flags);
	g_global_pts_base = base;
	g_local_pts_base = sys_get_local_cur_pts();
	if (init)
		g_global_pts_last = 0;
	osal_spin_unlock_irqrestore(&g_sys_spin_lock, &flags);
}

hi_u64 sys_get_time_stamp(hi_void)
{
	hi_u64 pts;
	hi_ulong flags = 0;

	osal_spin_lock_irqsave(&g_sys_spin_lock, &flags);
	pts = g_global_pts_base - g_local_pts_base + sys_get_local_cur_pts();
	if (pts < g_global_pts_last)
		pts = g_global_pts_last + 10U;
	g_global_pts_last = pts;
	osal_spin_unlock_irqrestore(&g_sys_spin_lock, &flags);
	return pts;
}

hi_s32 sys_get_time_zone(hi_s32 *time_zone)
{
	if (time_zone == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	*time_zone = g_sys_time_zone;
	return HI_SUCCESS;
}

hi_s32 sys_get_now_string(hi_u8 *now_time)
{
	osal_timeval_t tv;
	osal_rtc_time_t tm;
	hi_s32 ret;

	if (now_time == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;

	osal_gettimeofday(&tv);
	osal_rtc_time_to_tm((hi_ulong)tv.tv_sec, &tm);
	ret = snprintf_s((char *)now_time, 32U, 31U, "%04d-%02d-%02d %02d:%02d:%02d",
		tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec);
	return (ret < 0) ? HI_ERR_SYS_ERR : HI_SUCCESS;
}

hi_gps_info *sys_get_gps_info(hi_void)
{
	return &g_gps_info;
}

dvpp_set_csc_param *sys_set_csc_param_callback(hi_void)
{
	return &g_dvpp_set_csc_param;
}

dvpp_get_csc_param *sys_get_csc_param_callback(hi_void)
{
	return &g_dvpp_get_csc_param;
}

hi_u32 sys_drv_get_chip_version(hi_void)
{
	return 50334447U;
}

hi_s32 sys_drv_get_stride(hi_u32 width, hi_u32 *stride)
{
	if (stride == HI_NULL)
		return HI_FAILURE;
	*stride = (width + 31U) & ~31U;
	return HI_SUCCESS;
}

hi_s32 sys_drv_drv_ioctrl(hi_mpp_chn *mpp_chn, hi_u32 func_id, hi_void *io_args)
{
	if (mpp_chn == HI_NULL || io_args == HI_NULL)
		return HI_FAILURE;
	return (func_id == 106U) ? HI_SUCCESS : HI_FAILURE;
}

hi_s32 sys_drv_get_custom_code(hi_u32 *custom_code)
{
	if (custom_code != HI_NULL)
		*custom_code = 0;
	return HI_SUCCESS;
}

hi_void sys_drv_get_cmp_cfg(hi_void *video_frame, hi_void *y_cmp_cfg, hi_void *c_cmp_cfg, hi_void *cmp_mode_ex_param)
{
	(void)video_frame;
	(void)y_cmp_cfg;
	(void)c_cmp_cfg;
	(void)cmp_mode_ex_param;
}

hi_void sys_drv_get_vb_cfg(hi_void *info, hi_void *config)
{
	(void)info;
	(void)config;
}

hi_void sys_drv_get_vdec_buffer_cfg(hi_payload_type type, hi_void *info, hi_void *config)
{
	(void)type;
	(void)info;
	(void)config;
}

hi_void sys_drv_get_cmp_bayer_cfg(hi_void *sys_cmp_in, hi_void *bayer_cmp_cfg, hi_void *bayer_dcmp_cfg)
{
	(void)sys_cmp_in;
	(void)bayer_cmp_cfg;
	(void)bayer_dcmp_cfg;
}

hi_void sys_drv_get_cmp_3dnr_cfg(hi_void *sys_cmp_in, hi_void *y_cmp_cfg, hi_void *c_cmp_cfg)
{
	(void)sys_cmp_in;
	(void)y_cmp_cfg;
	(void)c_cmp_cfg;
}

hi_void sys_drv_get_dcmp_muv1_cfg(hi_void *sys_cmp_in, hi_void *cmp_out)
{
	(void)sys_cmp_in;
	(void)cmp_out;
}

hi_void sys_drv_get_cmp_tde_cfg(hi_void *sys_cmp_in, hi_void *cmp_out)
{
	(void)sys_cmp_in;
	(void)cmp_out;
}

hi_void sys_drv_get_cmp_seg_cfg(const hi_void *sys_cmp_in, hi_void *y_cmp_cfg, hi_void *c_cmp_cfg)
{
	(void)sys_cmp_in;
	(void)y_cmp_cfg;
	(void)c_cmp_cfg;
}

hi_s32 sys_drv_get_vpss_venc_wrap_buffer_line(hi_vpss_venc_wrap_param *wrap_param, hi_u32 *buf_line)
{
	(void)wrap_param;
	(void)buf_line;
	return HI_ERR_SYS_NOT_SUPPORT;
}

hi_s32 sys_drv_set_raw_frame_compress(hi_raw_frame_compress_param *compress_param)
{
	if (compress_param == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	g_raw_frame_compress = *compress_param;
	return HI_SUCCESS;
}

hi_s32 sys_drv_get_raw_frame_compress(hi_raw_frame_compress_param *compress_param)
{
	if (compress_param == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	*compress_param = g_raw_frame_compress;
	return HI_SUCCESS;
}

hi_void sys_drv_init_default_coef_level(hi_void)
{
	hi_u32 i;

	memset(&g_scale_info, 0, sizeof(g_scale_info));
	for (i = 0; i < SCALE_RANGE_BUTT; ++i) {
		g_scale_info.hor_lum_coeff_level[i] = (hi_coeff_level)osal_min(i, (hi_u32)(COEFF_LEVEL_BUTT - 1U));
		g_scale_info.hor_chr_coeff_level[i] = (hi_coeff_level)osal_min(i, (hi_u32)(COEFF_LEVEL_BUTT - 1U));
		g_scale_info.ver_lum_coeff_level[i] = (hi_coeff_level)osal_min(i, (hi_u32)(COEFF_LEVEL_BUTT - 1U));
		g_scale_info.ver_chr_coeff_level[i] = (hi_coeff_level)osal_min(i, (hi_u32)(COEFF_LEVEL_BUTT - 1U));
	}
}

hi_s32 sys_drv_malloc_scale_coef_buf(sys_scale_addr *scale_coef, hi_char *pch_name)
{
	hi_void *virt = HI_NULL;
	hi_s32 ret;

	(void)pch_name;
	if (scale_coef == HI_NULL)
		return HI_FAILURE;

	ret = media_kernel_mem_malloc(SYS_SCALE_BUF_LEN, SYS_SCALE_ALLOC_FLAGS, &virt);
	if (ret != HI_SUCCESS)
		return HI_FAILURE;

	scale_coef->scl_coeff_set = virt;
	scale_coef->scl_coeff_phy_addr = 0;
	scale_coef->scl_coeff_virt2_bus_offset = 0;
	return HI_SUCCESS;
}

hi_s32 sys_drv_free_scale_coef_buffer(hi_void)
{
	if (g_scale_coef_addr.scl_coeff_set != HI_NULL) {
		media_kernel_mem_free(g_scale_coef_addr.scl_coeff_set);
		g_scale_coef_addr.scl_coeff_set = HI_NULL;
		g_scale_coef_addr.scl_coeff_phy_addr = 0;
		g_scale_coef_addr.scl_coeff_virt2_bus_offset = 0;
	}
	return HI_SUCCESS;
}

hi_void sys_drv_init_scale_coef(sys_drv_scl_coeff *scl_coeff_set, hi_bool deflicker)
{
	(void)deflicker;
	if (scl_coeff_set != HI_NULL)
		memset(scl_coeff_set, 0, sizeof(*scl_coeff_set));
}

hi_void *sys_drv_get_hor_lum_scale_coef(hi_u32 yhrat, sys_drv_scl_coeff *scl_coeff_set)
{
	if (scl_coeff_set == HI_NULL)
		return HI_NULL;
	if (yhrat >= 60U) return scl_coeff_set->hor_luma_coef7_00;
	if (yhrat >= 57U) return scl_coeff_set->hor_luma_coef6_75;
	if (yhrat >= 53U) return scl_coeff_set->hor_luma_coef6_50;
	if (yhrat >= 51U) return scl_coeff_set->hor_luma_coef6_25;
	if (yhrat >= 48U) return scl_coeff_set->hor_luma_coef6_00;
	if (yhrat >= 45U) return scl_coeff_set->hor_luma_coef5_75;
	if (yhrat >= 42U) return scl_coeff_set->hor_luma_coef5_50;
	if (yhrat >= 38U) return scl_coeff_set->hor_luma_coef5_25;
	if (yhrat >= 35U) return scl_coeff_set->hor_luma_coef5_00;
	if (yhrat >= 33U) return scl_coeff_set->hor_luma_coef4_50;
	if (yhrat >= 29U) return scl_coeff_set->hor_luma_coef4_00;
	if (yhrat >= 24U) return scl_coeff_set->hor_luma_coef3_75;
	if (yhrat >= 19U) return scl_coeff_set->hor_luma_coef3_375;
	if (yhrat >= 15U) return scl_coeff_set->hor_luma_coef3_00;
	if (yhrat >= 10U) return scl_coeff_set->hor_luma_coef2_50;
	if (yhrat >= 8U) return scl_coeff_set->hor_luma_coef2_00;
	return scl_coeff_set->hor_luma_coef1_50;
}

hi_void *sys_drv_get_hor_chr_scale_coef(hi_u32 chrat, sys_drv_scl_coeff *scl_coeff_set)
{
	if (scl_coeff_set == HI_NULL)
		return HI_NULL;
	if (chrat >= 60U) return scl_coeff_set->hor_chroma_coef7_00;
	if (chrat >= 57U) return scl_coeff_set->hor_chroma_coef6_75;
	if (chrat >= 53U) return scl_coeff_set->hor_chroma_coef6_50;
	if (chrat >= 51U) return scl_coeff_set->hor_chroma_coef6_25;
	if (chrat >= 48U) return scl_coeff_set->hor_chroma_coef6_00;
	if (chrat >= 45U) return scl_coeff_set->hor_chroma_coef5_75;
	if (chrat >= 42U) return scl_coeff_set->hor_chroma_coef5_50;
	if (chrat >= 38U) return scl_coeff_set->hor_chroma_coef5_25;
	if (chrat >= 35U) return scl_coeff_set->hor_chroma_coef5_00;
	if (chrat >= 33U) return scl_coeff_set->hor_chroma_coef4_50;
	if (chrat >= 29U) return scl_coeff_set->hor_chroma_coef4_00;
	if (chrat >= 24U) return scl_coeff_set->hor_chroma_coef3_75;
	if (chrat >= 19U) return scl_coeff_set->hor_chroma_coef3_375;
	if (chrat >= 15U) return scl_coeff_set->hor_chroma_coef3_00;
	if (chrat >= 10U) return scl_coeff_set->hor_chroma_coef2_50;
	if (chrat >= 8U) return scl_coeff_set->hor_chroma_coef2_00;
	return scl_coeff_set->hor_chroma_coef1_50;
}

hi_void *sys_drv_get_ver_lum_scale_coef(hi_u32 yvrat, sys_drv_scl_coeff *scl_coeff_set)
{
	if (scl_coeff_set == HI_NULL)
		return HI_NULL;
	if (yvrat >= 60U) return scl_coeff_set->ver_luma_coef7_00;
	if (yvrat >= 57U) return scl_coeff_set->ver_luma_coef6_75;
	if (yvrat >= 53U) return scl_coeff_set->ver_luma_coef6_50;
	if (yvrat >= 51U) return scl_coeff_set->ver_luma_coef6_25;
	if (yvrat >= 48U) return scl_coeff_set->ver_luma_coef6_00;
	if (yvrat >= 45U) return scl_coeff_set->ver_luma_coef5_75;
	if (yvrat >= 42U) return scl_coeff_set->ver_luma_coef5_50;
	if (yvrat >= 38U) return scl_coeff_set->ver_luma_coef5_25;
	if (yvrat >= 35U) return scl_coeff_set->ver_luma_coef5_00;
	if (yvrat >= 33U) return scl_coeff_set->ver_luma_coef4_50;
	if (yvrat >= 29U) return scl_coeff_set->ver_luma_coef4_00;
	if (yvrat >= 24U) return scl_coeff_set->ver_luma_coef3_75;
	if (yvrat >= 19U) return scl_coeff_set->ver_luma_coef3_375;
	if (yvrat >= 15U) return scl_coeff_set->ver_luma_coef3_00;
	if (yvrat >= 10U) return scl_coeff_set->ver_luma_coef2_50;
	if (yvrat >= 8U) return scl_coeff_set->ver_luma_coef2_00;
	return scl_coeff_set->ver_luma_coef1_50;
}

hi_void *sys_drv_get_ver_chr_scale_coef(hi_u32 cvrat, sys_drv_scl_coeff *scl_coeff_set)
{
	if (scl_coeff_set == HI_NULL)
		return HI_NULL;
	if (cvrat >= 60U) return scl_coeff_set->ver_chroma_coef7_00;
	if (cvrat >= 57U) return scl_coeff_set->ver_chroma_coef6_75;
	if (cvrat >= 53U) return scl_coeff_set->ver_chroma_coef6_50;
	if (cvrat >= 51U) return scl_coeff_set->ver_chroma_coef6_25;
	if (cvrat >= 48U) return scl_coeff_set->ver_chroma_coef6_00;
	if (cvrat >= 45U) return scl_coeff_set->ver_chroma_coef5_75;
	if (cvrat >= 42U) return scl_coeff_set->ver_chroma_coef5_50;
	if (cvrat >= 38U) return scl_coeff_set->ver_chroma_coef5_25;
	if (cvrat >= 35U) return scl_coeff_set->ver_chroma_coef5_00;
	if (cvrat >= 33U) return scl_coeff_set->ver_chroma_coef4_50;
	if (cvrat >= 29U) return scl_coeff_set->ver_chroma_coef4_00;
	if (cvrat >= 24U) return scl_coeff_set->ver_chroma_coef3_75;
	if (cvrat >= 19U) return scl_coeff_set->ver_chroma_coef3_375;
	if (cvrat >= 15U) return scl_coeff_set->ver_chroma_coef3_00;
	if (cvrat >= 10U) return scl_coeff_set->ver_chroma_coef2_50;
	if (cvrat >= 8U) return scl_coeff_set->ver_chroma_coef2_00;
	return scl_coeff_set->ver_chroma_coef1_50;
}

hi_s32 sys_drv_scale_coef_init(hi_void)
{
	hi_s32 ret;

	sys_drv_init_default_coef_level();
	ret = sys_drv_malloc_scale_coef_buf(&g_scale_coef_addr, "sys_scale_coef");
	if (ret == HI_SUCCESS)
		sys_drv_init_scale_coef(g_scale_coef_addr.scl_coeff_set, HI_FALSE);
	return ret;
}

hi_s32 sys_drv_scale_coef_exit(hi_void)
{
	return sys_drv_free_scale_coef_buffer();
}

hi_s32 sys_drv_init(hi_void)
{
	return sys_drv_scale_coef_init();
}

hi_s32 sys_drv_exit(hi_void)
{
	return sys_drv_scale_coef_exit();
}

hi_void sys_get_scale_coef_kva(hi_void **scale_coef_addr, hi_u64 *coef_len)
{
	if (scale_coef_addr != HI_NULL)
		*scale_coef_addr = g_scale_coef_addr.scl_coeff_set;
	if (coef_len != HI_NULL)
		*coef_len = SYS_SCALE_BUF_LEN;
}

hi_u32 sys_get_scale_coef(hi_mod_id mod_id, hi_void *scale_coef_opt_ptr, hi_void *pv_rate, hi_void *pv_cfg)
{
	scale_coef_opt *opt = scale_coef_opt_ptr;
	sys_scl_rate *rate = pv_rate;
	hi_u64 *cfg = pv_cfg;
	hi_void *hor_lum;
	hi_void *ver_lum;
	hi_void *hor_chr;
	hi_void *ver_chr;
	hi_u8 *base;

	(void)mod_id;
	if (opt == HI_NULL || rate == HI_NULL || cfg == HI_NULL || g_scale_coef_addr.scl_coeff_set == HI_NULL)
		return (hi_u32)-1;

	base = (hi_u8 *)g_scale_coef_addr.scl_coeff_set;
	switch (opt->ive_sclcoef_mode) {
	case SCLCOEF_IVE_TAP4:
		hor_lum = g_scale_coef_addr.scl_coeff_set->ycoefficient4_cubic;
		ver_lum = g_scale_coef_addr.scl_coeff_set->ycoefficient4_cubic;
		hor_chr = g_scale_coef_addr.scl_coeff_set->ccoefficient4_cubic;
		ver_chr = g_scale_coef_addr.scl_coeff_set->ccoefficient4_cubic;
		break;
	case SCLCOEF_IVE_TAP6:
		hor_lum = g_scale_coef_addr.scl_coeff_set->coefficient6_cubic;
		ver_lum = g_scale_coef_addr.scl_coeff_set->coefficient6_cubic;
		hor_chr = g_scale_coef_addr.scl_coeff_set->coefficient6_cubic;
		ver_chr = g_scale_coef_addr.scl_coeff_set->coefficient6_cubic;
		break;
	case SCLCOEF_IVE_TAP8:
		hor_lum = g_scale_coef_addr.scl_coeff_set->coefficient8_cubic;
		ver_lum = g_scale_coef_addr.scl_coeff_set->coefficient8_cubic;
		hor_chr = g_scale_coef_addr.scl_coeff_set->coefficient8_cubic;
		ver_chr = g_scale_coef_addr.scl_coeff_set->coefficient8_cubic;
		break;
	case SCLCOEF_DIS_LINEAR:
		hor_lum = g_scale_coef_addr.scl_coeff_set->coefficient8_lanczos2_8tap_dis;
		ver_lum = g_scale_coef_addr.scl_coeff_set->coefficient6_4_m_a20_dis;
		hor_chr = g_scale_coef_addr.scl_coeff_set->coefficient8_lanczos2_8tap_dis;
		ver_chr = g_scale_coef_addr.scl_coeff_set->coefficient6_4_m_a20_dis;
		break;
	case SCLCOEF_NORMAL:
	default:
		hor_lum = sys_drv_get_hor_lum_scale_coef(rate->yhrat, g_scale_coef_addr.scl_coeff_set);
		ver_lum = sys_drv_get_ver_lum_scale_coef(rate->yvrat, g_scale_coef_addr.scl_coeff_set);
		hor_chr = sys_drv_get_hor_chr_scale_coef(rate->chrat, g_scale_coef_addr.scl_coeff_set);
		ver_chr = sys_drv_get_ver_chr_scale_coef(rate->cvrat, g_scale_coef_addr.scl_coeff_set);
		break;
	}

	cfg[4] = (hi_u64)(uintptr_t)hor_lum;
	cfg[5] = (hi_u64)(uintptr_t)ver_lum;
	cfg[6] = (hi_u64)(uintptr_t)hor_chr;
	cfg[7] = (hi_u64)(uintptr_t)ver_chr;
	cfg[0] = (hi_u64)(uintptr_t)hor_lum - (hi_u64)(uintptr_t)base;
	cfg[1] = (hi_u64)(uintptr_t)ver_lum - (hi_u64)(uintptr_t)base;
	cfg[2] = (hi_u64)(uintptr_t)hor_chr - (hi_u64)(uintptr_t)base;
	cfg[3] = (hi_u64)(uintptr_t)ver_chr - (hi_u64)(uintptr_t)base;
	return HI_SUCCESS;
}

static hi_void sys_init_core_usable_info(hi_void)
{
	hi_u32 i;

	for (i = 0; i < DVPP_IP_TYPE_BUTT; ++i) {
		g_core_usable_info[i].usable_mode = CORE_FULL_GOOD;
		g_core_usable_info[i].core_max_num = 64U;
		g_core_usable_info[i].core_usable_num = 64U;
		g_core_usable_info[i].core_is_usable = ~0ULL;
	}
}

static hi_void sys_drv_read_chip_info(hi_void)
{
	hi_void *platform_reg;
	hi_void *chip_reg;
	hi_u32 platform_val = 0;
	hi_u32 chip_val = 0;
	hi_u32 chip_code;
	hi_u32 chip_rev;

	platform_reg = osal_ioremap(0xC01001FCUL, sizeof(hi_u32));
	if (platform_reg != HI_NULL) {
		platform_val = osal_readl(platform_reg);
		osal_iounmap(platform_reg);
	}

	if (platform_val == 0U)
		g_sys_platform_type = SYS_PLAT_ASIC;
	else if (((platform_val >> 16) & 0xFFFFU) == 0U)
		g_sys_platform_type = SYS_PLAT_FPGA;
	else if (((platform_val >> 16) & 0xFFFFU) == 1U)
		g_sys_platform_type = SYS_PLAT_EMU;
	else if (((platform_val >> 16) & 0xFFFFU) == 2U)
		g_sys_platform_type = SYS_PLAT_ESL;

	chip_reg = osal_ioremap(0x80020018UL, sizeof(hi_u32));
	if (chip_reg != HI_NULL) {
		chip_val = osal_readl(chip_reg);
		osal_iounmap(chip_reg);
	}
	if (chip_val == 0U) {
		chip_reg = osal_ioremap(0xC01001F8UL, sizeof(hi_u32));
		if (chip_reg != HI_NULL) {
			chip_val = osal_readl(chip_reg);
			osal_iounmap(chip_reg);
		}
	}

	chip_code = chip_val >> 4;
	chip_rev = chip_val & 0xFU;
	if (chip_code == 6481U) {
		if (chip_rev == 1U)
			g_sys_chip_type = SYS_CHIP_MDC_V1;
		else if (chip_rev == 2U)
			g_sys_chip_type = SYS_CHIP_MDC_V2;
		else if (chip_rev == 3U)
			g_sys_chip_type = SYS_CHIP_MDC_V51_LITE;
	} else if (chip_code == 6529U) {
		g_sys_chip_type = SYS_CHIP_CLOUD_V2;
	} else if (chip_code == 6417U) {
		g_sys_chip_type = SYS_CHIP_MINI_V2;
	}

#if defined(CFG_SOC_PLATFORM_MINIV3) && defined(CFG_FEATURE_RC_MODE)
	g_sys_chip_type = SYS_CHIP_MINI_V2_MDC;
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V11
	g_sys_chip_type = SYS_CHIP_MINI_V2_MDC;
#endif
}

static hi_s32 sys_pg_info_thread(void *data)
{
	(void)data;
	if (g_drval_check_pg_info_support != HI_NULL)
		(void)g_drval_check_pg_info_support();
	osal_complete(&pg_info_event);
	return 0;
}

static hi_void sys_init_ip_num(hi_void)
{
	switch (g_sys_chip_type) {
	case SYS_CHIP_MDC_V2:
		g_vdec_ip_num_per_device = 3U;
		g_jpegd_ip_num_per_device = 1U;
		break;
	case SYS_CHIP_CLOUD_V2:
		g_vdec_ip_num_per_device = 2U;
		g_jpegd_ip_num_per_device = 28U;
		break;
	case SYS_CHIP_MINI_V2:
	case SYS_CHIP_MINI_V2_MDC:
		g_vdec_ip_num_per_device = 4U;
		g_jpegd_ip_num_per_device = 2U;
		break;
	default:
		g_vdec_ip_num_per_device = 12U;
		g_jpegd_ip_num_per_device = 16U;
		break;
	}

	g_vdec_num = g_vdec_ip_num_per_device * g_device_num;
	g_jpegd_num = g_jpegd_ip_num_per_device * g_device_num;
}

static hi_void sys_exit_mem_conf(hi_void)
{
	hi_u32 i;

	for (i = 0; i < SYS_MEM_CTX_NUM; ++i) {
		if (g_mem_ctx[i].mmz_names != HI_NULL) {
			osal_kfree(g_mem_ctx[i].mmz_names);
			g_mem_ctx[i].mmz_names = HI_NULL;
		}
		g_mem_ctx[i].dev_cnt = 0;
		g_mem_ctx[i].chn_cnt = 0;
	}
}

hi_u32 vdec_get_ip_num_per_device(hi_void)
{
	return g_vdec_ip_num_per_device;
}
EXPORT_SYMBOL(vdec_get_ip_num_per_device);

hi_u32 vdec_get_ip_num(hi_void)
{
	return g_vdec_num;
}

hi_u32 jpegd_get_ip_num_per_device(hi_void)
{
	return g_jpegd_ip_num_per_device;
}
EXPORT_SYMBOL(jpegd_get_ip_num_per_device);

hi_u32 jpege_get_ip_num_per_device(hi_void)
{
	hi_u32 limited;

	osal_spin_lock(&g_sys_func_spin_lock);
	if (g_dvpp_jpege_get_limted_core_num == HI_NULL) {
		osal_spin_unlock(&g_sys_func_spin_lock);
		return 0;
	}
	limited = g_dvpp_jpege_get_limted_core_num();
	osal_spin_unlock(&g_sys_func_spin_lock);

	if (g_device_num == 0U)
		return 0;
	return limited / g_device_num;
}
EXPORT_SYMBOL(jpege_get_ip_num_per_device);

hi_u32 jpegd_get_ip_num(hi_void)
{
	return g_jpegd_num;
}
EXPORT_SYMBOL(jpegd_get_ip_num);

sys_platform_type sys_drv_get_platform_type(hi_void)
{
	return g_sys_platform_type;
}

sys_chip_type sys_drv_get_chip_type(hi_void)
{
	return g_sys_chip_type;
}

hi_u32 sys_get_ip_num_per_device(DVPP_IP_TYPE ip_type)
{
	if (ip_type != DVPP_IP_TYPE_VPC)
		return 0;
	if (g_sys_chip_type == SYS_CHIP_MINI_V2 || g_sys_chip_type == SYS_CHIP_MINI_V2_MDC)
		return 2U;
	if (g_sys_chip_type == SYS_CHIP_CLOUD_V2)
		return 10U;
	if (g_sys_chip_type == SYS_CHIP_MDC_V2 || g_sys_chip_type == SYS_CHIP_MDC_V51_LITE)
		return 4U;
	return 12U;
}

hi_u32 sys_get_ip_num(DVPP_IP_TYPE ip_type)
{
	return sys_get_ip_num_per_device(ip_type) * g_device_num;
}

hi_s32 sys_get_core_usable_info(DVPP_IP_TYPE core_type, core_usable_info *usable_info)
{
	if (usable_info == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	if ((hi_u32)core_type >= DVPP_IP_TYPE_BUTT)
		return HI_ERR_SYS_ILLEGAL_PARAM;
	*usable_info = g_core_usable_info[core_type];
	return HI_SUCCESS;
}

hi_void sys_export_func_spin_lock(hi_void)
{
	osal_spin_lock(&g_sys_func_spin_lock);
}

hi_void sys_export_func_spin_unlock(hi_void)
{
	osal_spin_unlock(&g_sys_func_spin_lock);
}

hi_s32 sys_print_proc_title(osal_proc_entry_t *entry, const hi_char *format, ...)
{
	hi_char title[64];
	hi_char line[SYS_PROC_TITLE_WIDTH];
	hi_u32 len;
	hi_u32 start = SYS_PROC_TITLE_OFFSET;
	hi_s32 ret;
	va_list args;

	if (entry == HI_NULL || format == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;

	memset(line, '-', sizeof(line));
	line[sizeof(line) - 1U] = '\0';

	va_start(args, format);
	ret = vsnprintf_s(title, sizeof(title), sizeof(title) - 1U, format, args);
	va_end(args);
	if (ret < 0)
		return HI_ERR_SYS_ERR;

	len = (hi_u32)osal_strlen(title);
	if (start + len >= sizeof(line))
		start = (sizeof(line) - 1U > len) ? (sizeof(line) - 1U - len) : 0U;
	if (len > 0U && start < sizeof(line) - 1U)
		(void)memcpy_s(&line[start], sizeof(line) - start, title, osal_min((size_t)len, sizeof(line) - start - 1U));

	osal_seq_printf(entry, "\n%s\n", line);
	return HI_SUCCESS;
}

static hi_s32 sys_proc_show_ex(const osal_proc_entry_t *entry)
{
	hi_u32 i;
	fn_sys_proc_show *proc_show;

	if (entry == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;

	osal_seq_printf(entry, "[SYS] Version: [HiDVPP]\n");

	osal_spin_lock(&g_sys_func_spin_lock);
	proc_show = g_sys_dvpp_proc_show_func;
	osal_spin_unlock(&g_sys_func_spin_lock);
	if (proc_show != HI_NULL) {
		osal_seq_printf(entry, "PG core map ...\n");
		proc_show(entry);
	}

	if (g_runtime_state == SYS_RUNTIME_READY) {
		osal_seq_printf(entry, "state: run\n");
		for (i = 0; i < SYS_MEM_CTX_NUM; ++i) {
			if (g_mem_ctx[i].dev_cnt == 0U || g_mem_ctx[i].chn_cnt == 0U)
				continue;
			osal_seq_printf(entry, "mem_ctx[%u]: dev=%u chn=%u\n",
				i, g_mem_ctx[i].dev_cnt, g_mem_ctx[i].chn_cnt);
		}
	}
	return HI_SUCCESS;
}

hi_void sys_regist_dvpp_proc_show(fn_sys_proc_show *func)
{
	osal_spin_lock(&g_sys_func_spin_lock);
	g_sys_dvpp_proc_show_func = func;
	osal_spin_unlock(&g_sys_func_spin_lock);
}

hi_void sys_unregist_dvpp_proc_show(hi_void)
{
	osal_spin_lock(&g_sys_func_spin_lock);
	g_sys_dvpp_proc_show_func = HI_NULL;
	osal_spin_unlock(&g_sys_func_spin_lock);
}

hi_void sys_regist_dvpp_dump_info(fn_sys_dump_info *func)
{
	osal_spin_lock(&g_sys_func_spin_lock);
	g_sys_dvpp_dump_info_func = func;
	osal_spin_unlock(&g_sys_func_spin_lock);
}

hi_void sys_unregist_dvpp_dump_info(hi_void)
{
	osal_spin_lock(&g_sys_func_spin_lock);
	g_sys_dvpp_dump_info_func = HI_NULL;
	osal_spin_unlock(&g_sys_func_spin_lock);
}

hi_s32 sys_get_mmz_name(hi_mpp_chn *chn, hi_void **pp_mmz_name)
{
	struct sys_mem_ctx *ctx;
	hi_u32 idx;

	if (chn == HI_NULL || pp_mmz_name == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	if ((hi_u32)chn->mod_id >= SYS_MEM_CTX_NUM)
		return HI_ERR_SYS_ILLEGAL_PARAM;

	ctx = &g_mem_ctx[chn->mod_id];
	if (ctx->dev_cnt == 0U || ctx->chn_cnt == 0U || ctx->mmz_names == HI_NULL)
		return HI_ERR_SYS_UNEXIST;
	if (chn->dev_id < 0 || chn->chn_id < 0)
		return HI_ERR_SYS_ILLEGAL_PARAM;
	if ((hi_u32)chn->dev_id >= ctx->dev_cnt || (hi_u32)chn->chn_id >= ctx->chn_cnt)
		return HI_ERR_SYS_ILLEGAL_PARAM;

	idx = (hi_u32)chn->dev_id * ctx->chn_cnt + (hi_u32)chn->chn_id;
	*pp_mmz_name = ctx->mmz_names[idx];
	return HI_SUCCESS;
}

hi_s32 sys_get_mem_ddr_name(const hi_mpp_chn *chn, mpp_sys_ddr_name *mem_ddr)
{
	struct sys_mem_ctx *ctx;
	hi_u32 i;
	hi_u32 used = 1U;

	if (chn == HI_NULL || mem_ddr == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	if ((hi_u32)chn->mod_id >= SYS_MEM_CTX_NUM)
		return HI_ERR_SYS_ILLEGAL_PARAM;

	ctx = &g_mem_ctx[chn->mod_id];
	if (ctx->dev_cnt == 0U || ctx->chn_cnt == 0U || ctx->mmz_names == HI_NULL)
		return HI_ERR_SYS_UNEXIST;

	memset(mem_ddr, 0, sizeof(*mem_ddr));
	mem_ddr->num = 1U;
	mem_ddr->mmz_name[0] = HI_NULL;
	for (i = 0; i < ctx->dev_cnt * ctx->chn_cnt && used < SYS_DDR_MAXNUM; ++i) {
		hi_void *name = ctx->mmz_names[i];
		hi_u32 j;
		hi_bool exists = HI_FALSE;

		if (name == HI_NULL)
			continue;
		for (j = 1U; j < used; ++j) {
			if (mem_ddr->mmz_name[j] == name) {
				exists = HI_TRUE;
				break;
			}
		}
		if (exists == HI_FALSE)
			mem_ddr->mmz_name[used++] = name;
	}
	mem_ddr->num = used;
	return HI_SUCCESS;
}

hi_s32 sys_set_device_num(hi_u32 device_num)
{
	g_device_num = device_num;
	sys_init_ip_num();
	return HI_SUCCESS;
}

hi_u32 sys_get_die_num_per_device(hi_void)
{
	return g_die_num;
}

hi_s32 sys_get_cpu_info(hi_u32 device_id, hi_u32 *cpu_core_num, hi_u32 *cpu_bitmap)
{
	hi_u32 online;
	hi_u32 bitmap;

	if (cpu_core_num == HI_NULL || cpu_bitmap == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	if (device_id != 0U)
		return HI_ERR_SYS_ILLEGAL_PARAM;
	if (!(g_sys_chip_type == SYS_CHIP_DC || g_sys_chip_type == SYS_CHIP_CLOUD_V2 ||
	      g_sys_chip_type == SYS_CHIP_MINI_V2 || g_sys_chip_type == SYS_CHIP_MINI_V2_MDC))
		return HI_ERR_SYS_NOT_SUPPORT;

	online = sys_online_cpu_count();
	bitmap = sys_default_cpu_bitmap();
	*cpu_core_num = online;
	*cpu_bitmap = bitmap;
	return HI_SUCCESS;
}

hi_u32 sys_get_cpu_id_by_index(hi_u32 device_id, hi_u32 idx, CPU_TYPE get_cpu_type)
{
	hi_u32 cpu_num = 0;
	hi_u32 cpu_bitmap = 0;
	hi_u32 cpu;
	hi_u32 count = 0;

	(void)get_cpu_type;
	if (sys_get_cpu_info(device_id, &cpu_num, &cpu_bitmap) != HI_SUCCESS || cpu_num == 0U)
		return 0;

	for (cpu = 0; cpu < 32U; ++cpu) {
		if (((cpu_bitmap >> cpu) & 0x1U) == 0U)
			continue;
		if (count == (idx % cpu_num))
			return cpu;
		count++;
	}
	return 0;
}

hi_s32 sys_get_cpu_mask(hi_u32 device_id, hi_void *mask, CPU_TYPE get_cpu_type)
{
	struct cpumask *cpu_mask = mask;
	hi_u32 cpu_num = 0;
	hi_u32 cpu_bitmap = 0;
	hi_u32 cpu;
	hi_s32 ret;

	(void)get_cpu_type;
	if (cpu_mask == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;

	ret = sys_get_cpu_info(device_id, &cpu_num, &cpu_bitmap);
	if (ret != HI_SUCCESS)
		return ret;

	cpumask_clear(cpu_mask);
	for (cpu = 0; cpu < 32U; ++cpu) {
		if (((cpu_bitmap >> cpu) & 0x1U) != 0U)
			cpumask_set_cpu(cpu, cpu_mask);
	}
	return HI_SUCCESS;
}

hi_s32 sys_set_vf_and_vfg_aicpu_bitmap(hi_u32 device_id, hi_u32 vf_id,
	hi_u32 vf_aicpu_bitmap, hi_u32 vfg_aicpu_bitmap)
{
	if (device_id != 0U || vf_id >= SYS_VF_BITMAP_NUM)
		return HI_ERR_SYS_ILLEGAL_PARAM;
	g_dvpp_vf_bitmap[vf_id].vf_aicpu_bitmap = vf_aicpu_bitmap;
	g_dvpp_vf_bitmap[vf_id].vfg_aicpu_bitmap = vfg_aicpu_bitmap;
	return HI_SUCCESS;
}

hi_s32 sys_get_vf_or_vfg_aicpu_bitmap(hi_u32 device_id, hi_u32 vf_id, hi_u32 *aicpu_bitmap, hi_u32 is_vfg)
{
	if (aicpu_bitmap == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	if (device_id != 0U || vf_id >= SYS_VF_BITMAP_NUM)
		return HI_ERR_SYS_ILLEGAL_PARAM;
	*aicpu_bitmap = is_vfg ? g_dvpp_vf_bitmap[vf_id].vfg_aicpu_bitmap :
		g_dvpp_vf_bitmap[vf_id].vf_aicpu_bitmap;
	return HI_SUCCESS;
}

hi_s32 sys_set_aicpu_bind_index(hi_u32 device_id, hi_u32 vf_id)
{
	hi_u32 cpu_num = 0;
	hi_u32 cpu_bitmap = 0;
	hi_u32 cpu;
	hi_u32 idx = 0;
	hi_u32 bind_bitmap;
	hi_s32 ret;

	if (device_id != 0U || vf_id >= SYS_VF_BITMAP_NUM)
		return HI_ERR_SYS_ILLEGAL_PARAM;

	ret = sys_get_cpu_info(device_id, &cpu_num, &cpu_bitmap);
	if (ret != HI_SUCCESS)
		return ret;

	bind_bitmap = g_dvpp_vf_bitmap[vf_id].vf_aicpu_bitmap;
	memset(g_dvpp_vf_bitmap[vf_id].vf_aicpu_bind_index, 0, sizeof(g_dvpp_vf_bitmap[vf_id].vf_aicpu_bind_index));
	for (cpu = 0; cpu < 32U && idx < MAX_CPU_NUM_PER_DEVICE; ++cpu) {
		if (((bind_bitmap >> cpu) & 0x1U) == 0U)
			continue;
		g_dvpp_vf_bitmap[vf_id].vf_aicpu_bind_index[idx++] = cpu;
	}
	g_dvpp_vf_bitmap[vf_id].vf_aicpu_num = idx;
	return HI_SUCCESS;
}

hi_s32 sys_get_vf_or_vfg_mask(hi_u32 device_id, hi_u32 vf_id, hi_void *mask, hi_u32 is_vfg)
{
	struct cpumask *cpu_mask = mask;
	hi_u32 bitmap;
	hi_u32 cpu;
	hi_s32 ret;

	if (cpu_mask == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	ret = sys_get_vf_or_vfg_aicpu_bitmap(device_id, vf_id, &bitmap, is_vfg);
	if (ret != HI_SUCCESS)
		return ret;

	cpumask_clear(cpu_mask);
	for (cpu = 0; cpu < 32U; ++cpu) {
		if (((bitmap >> cpu) & 0x1U) != 0U)
			cpumask_set_cpu(cpu, cpu_mask);
	}
	return HI_SUCCESS;
}

hi_u32 sys_get_irq_aicpu_id_by_index(hi_u32 device_id, hi_u32 vf_id, hi_u32 idx)
{
	if (device_id != 0U || vf_id >= SYS_VF_BITMAP_NUM)
		return 0;
	if (g_dvpp_vf_bitmap[vf_id].vf_aicpu_num == 0U)
		(void)sys_set_aicpu_bind_index(device_id, vf_id);
	if (g_dvpp_vf_bitmap[vf_id].vf_aicpu_num == 0U)
		return sys_get_cpu_id_by_index(device_id, idx, CPU_TYPE_AI);
	return g_dvpp_vf_bitmap[vf_id].vf_aicpu_bind_index[idx % g_dvpp_vf_bitmap[vf_id].vf_aicpu_num];
}

hi_s32 sys_get_aicpu_bitmap(hi_u32 ext_phy_dev_id, hi_u32 *cpu_bitmap)
{
	hi_u32 cpu_num = 0;

	if (cpu_bitmap == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	if (!(g_sys_chip_type == SYS_CHIP_CLOUD_V2 || g_sys_chip_type == SYS_CHIP_MINI_V2 ||
	      g_sys_chip_type == SYS_CHIP_MINI_V2_MDC))
		return HI_ERR_SYS_NOT_SUPPORT;
	return sys_get_cpu_info(ext_phy_dev_id, &cpu_num, cpu_bitmap);
}

hi_s32 sys_bind_irq_to_cpu(dvpp_core_info core_info, hi_u32 cpu_id)
{
	if (cpu_id >= sys_online_cpu_count())
		return HI_ERR_SYS_ILLEGAL_PARAM;
	if ((hi_u32)core_info.ip_type >= DVPP_IP_TYPE_BUTT ||
		(hi_u32)core_info.irq_type >= IRQ_TYPE_BUTT ||
		core_info.core_id >= DVPP_MAX_IP_NUM_PER_DEVICE)
		return HI_ERR_SYS_ILLEGAL_PARAM;

	cpumask_clear(&g_irq_cpumask[core_info.ip_type][core_info.irq_type][core_info.core_id]);
	cpumask_set_cpu(cpu_id, &g_irq_cpumask[core_info.ip_type][core_info.irq_type][core_info.core_id]);
	return osal_irq_set_affinity_hint(core_info.irq,
		&g_irq_cpumask[core_info.ip_type][core_info.irq_type][core_info.core_id]);
}

hi_s32 sys_bind_irq_to_cpu_mask(hi_u32 irq, const hi_void *mask)
{
	if (mask == HI_NULL)
		return HI_ERR_SYS_NULL_PTR;
	return osal_irq_set_affinity_hint(irq, mask);
}

hi_s32 sys_unbind_irq_to_cpu(hi_u32 irq)
{
	return osal_irq_set_affinity_hint(irq, HI_NULL);
}

hi_void sys_bootdot_init_block(hi_u64 exception_id, PROCESS_TYPE process_type, hi_u8 core_id, hi_u8 status)
{
	if (!sys_chip_is_mini_v2_family())
		return;
	if (osal_bootdot_init_blk(SYS_BOOTDOT_BLOCK_ID, 0U, (hi_u32)exception_id,
		transform_block_status(process_type, core_id, status)) == HI_SUCCESS)
		g_bootdot_init_ok = HI_TRUE;
}

hi_void sys_bootdot_set_block_status(PROCESS_TYPE process_type, hi_u8 core_id, hi_u8 status)
{
	if (!sys_chip_is_mini_v2_family() || g_bootdot_init_ok == HI_FALSE)
		return;
	(void)osal_bootdot_set_blk(SYS_BOOTDOT_BLOCK_ID, 0U,
		transform_block_status(process_type, core_id, status));
}

static hi_s32 sys_user_exit(hi_void)
{
	hi_s32 ret;
	hi_s32 user_cnt;
	hi_s32 retry;

	ret = osal_down(&g_sys_sem);
	if (ret != HI_SUCCESS)
		return -ERESTARTSYS;

	if (osal_atomic_read(&g_sys_open_cnt) == 0) {
		osal_up(&g_sys_sem);
		return HI_SUCCESS;
	}

	user_cnt = osal_atomic_dec_return(&g_sys_open_cnt);
	if (user_cnt != 0) {
		osal_up(&g_sys_sem);
		return HI_SUCCESS;
	}

	if (g_runtime_state == SYS_RUNTIME_IDLE) {
		osal_up(&g_sys_sem);
		return HI_SUCCESS;
	}
	if (g_runtime_state == SYS_RUNTIME_BLOCKED) {
		osal_up(&g_sys_sem);
		return HI_ERR_SYS_BUSY;
	}

	cmpi_stop_modules();
	for (retry = 10; retry > 0; --retry) {
		if (cmpi_query_modules() == HI_SUCCESS)
			break;
		osal_msleep_uninterrupt(50);
	}
	if (retry == 0) {
		osal_up(&g_sys_sem);
		return HI_ERR_SYS_BUSY;
	}

	osal_msleep_uninterrupt(50);
	cmpi_exit_modules();
	(void)sys_drv_exit();
	g_conf = HI_FALSE;
	memset(&g_sys_config, 0, sizeof(g_sys_config));
	g_runtime_state = SYS_RUNTIME_IDLE;
	g_sys_state = SYS_RUNTIME_IDLE;
	module_put(THIS_MODULE);
	osal_up(&g_sys_sem);
	return HI_SUCCESS;
}

static hi_s32 sys_do_init(hi_void *arg)
{
	(void)arg;
	return HI_SUCCESS;
}

static hi_void sys_do_exit(hi_void)
{
}

static hi_void sys_notify(mod_notice_id notice_id)
{
	(void)notice_id;
}

static hi_void sys_query_state(mod_state *state)
{
	if (state == HI_NULL)
		return;
	*state = (osal_atomic_read(&g_sys_busy_cnt) != 0) ? MOD_STATE_BUSY : MOD_STATE_FREE;
}

static hi_u32 sys_get_ver_magic(hi_void)
{
	return VERSION_MAGIC;
}

static hi_s32 sys_open(hi_void *private_data)
{
	hi_s32 ret;
	hi_s32 user_cnt;

	ret = osal_down(&g_sys_sem);
	if (ret != HI_SUCCESS)
		return HI_FAILURE;

	if (osal_atomic_read(&g_sys_open_cnt) == 0)
		(void)try_module_get(THIS_MODULE);
	user_cnt = osal_atomic_inc_return(&g_sys_open_cnt);

	if (user_cnt == 1) {
		if (g_runtime_state == SYS_RUNTIME_IDLE) {
			hi_ulong flags = 0;

			osal_spin_lock_irqsave(&g_sys_spin_lock, &flags);
			g_global_pts_base = sys_get_local_cur_pts();
			g_local_pts_base = g_global_pts_base;
			osal_spin_unlock_irqrestore(&g_sys_spin_lock, &flags);

			ret = cmpi_init_modules();
			if (ret != HI_SUCCESS) {
				(void)osal_atomic_dec_return(&g_sys_open_cnt);
				module_put(THIS_MODULE);
				osal_up(&g_sys_sem);
				return HI_FAILURE;
			}

			ret = sys_drv_init();
			if (ret != HI_SUCCESS) {
				cmpi_exit_modules();
				(void)osal_atomic_dec_return(&g_sys_open_cnt);
				module_put(THIS_MODULE);
				osal_up(&g_sys_sem);
				return HI_FAILURE;
			}

			g_runtime_state = SYS_RUNTIME_READY;
			g_sys_state = SYS_RUNTIME_READY;
		} else if (g_runtime_state == SYS_RUNTIME_BLOCKED) {
			(void)osal_atomic_dec_return(&g_sys_open_cnt);
			module_put(THIS_MODULE);
			osal_up(&g_sys_sem);
			return HI_FAILURE;
		}
	}

	osal_up(&g_sys_sem);

	ret = create_sys_file_data(private_data);
	if (ret != HI_SUCCESS)
		(void)sys_user_exit();
	return ret;
}

static hi_s32 sys_close(hi_void *private_data)
{
	destroy_sys_file_data(private_data);
	return sys_user_exit();
}

static hi_slong sys_ioctl(hi_u32 cmd, hi_ulong arg, hi_void *private_data)
{
	hi_void *karg = (hi_void *)(uintptr_t)arg;
	struct sys_file_data *file_data = HI_NULL;
	hi_s32 ret = HI_FAILURE;

	if (private_data != HI_NULL)
		file_data = *(struct sys_file_data **)private_data;
	if (private_data == HI_NULL || file_data == HI_NULL)
		return HI_FAILURE;

	ret = osal_down_interruptible(&g_sys_ctx);
	if (ret != HI_SUCCESS)
		return -ERESTARTSYS;

	if (cmd == SYS_EXIT_CTRL) {
		osal_up(&g_sys_ctx);
		return HI_SUCCESS;
	}

	osal_atomic_inc_return(&g_sys_busy_cnt);
	switch (cmd) {
	case SYS_INIT_CTRL:
	case SYS_EXIT_CTRL:
		ret = HI_SUCCESS;
		break;
	case SYS_SET_CONFIG_CTRL:
		if (karg == HI_NULL) {
			ret = HI_ERR_SYS_NULL_PTR;
			break;
		}
		if (((hi_mpp_sys_config *)karg)->align > 0x400U) {
			ret = HI_ERR_SYS_ILLEGAL_PARAM;
			break;
		}
		g_sys_config = *(hi_mpp_sys_config *)karg;
		g_conf = HI_TRUE;
		ret = HI_SUCCESS;
		break;
	case SYS_GET_CONFIG_CTRL:
		if (karg == HI_NULL) {
			ret = HI_ERR_SYS_NULL_PTR;
			break;
		}
		ret = g_conf ? sys_copy_to_arg(karg, &g_sys_config, sizeof(g_sys_config)) : HI_ERR_SYS_BUSY;
		break;
	case SYS_INIT_PTSBASE:
		if (karg == HI_NULL) {
			ret = HI_ERR_SYS_NULL_PTR;
			break;
		}
		sys_sync_time_stamp(*(hi_u64 *)karg, HI_TRUE);
		ret = HI_SUCCESS;
		break;
	case SYS_SYNC_PTS:
		if (karg == HI_NULL) {
			ret = HI_ERR_SYS_NULL_PTR;
			break;
		}
		sys_sync_time_stamp(*(hi_u64 *)karg, HI_FALSE);
		ret = HI_SUCCESS;
		break;
	case SYS_GET_CURPTS:
		if (karg == HI_NULL) {
			ret = HI_ERR_SYS_NULL_PTR;
			break;
		}
		*(hi_u64 *)karg = sys_get_time_stamp();
		ret = HI_SUCCESS;
		break;
	case SYS_DUMP_INFO:
		if (karg == HI_NULL) {
			ret = HI_FAILURE;
			break;
		}
		if (g_sys_dvpp_dump_info_func != HI_NULL) {
			hi_void *buf = osal_vmalloc(SYS_DUMP_PROFILING_INFO_LEN);

			if (buf == HI_NULL) {
				ret = HI_FAILURE;
				break;
			}
			osal_spin_lock(&g_sys_func_spin_lock);
			if (g_sys_dvpp_dump_info_func != HI_NULL)
				ret = g_sys_dvpp_dump_info_func(*(hi_mod_id *)karg, buf, SYS_DUMP_PROFILING_INFO_LEN);
			else
				ret = HI_SUCCESS;
			osal_spin_unlock(&g_sys_func_spin_lock);
			osal_vfree(buf);
		} else {
			ret = HI_SUCCESS;
		}
		break;
	case SYS_BIND_CTRL:
		ret = sys_bind_chn(&((sys_bind_args *)karg)->src_chn, &((sys_bind_args *)karg)->dest_chn, file_data);
		break;
	case SYS_UNBIND_CTRL:
		ret = sys_unbind(&((sys_bind_args *)karg)->src_chn, &((sys_bind_args *)karg)->dest_chn);
		break;
	case SYS_GETBINDBYDEST:
		ret = sys_get_bind_by_dest(&((sys_bind_args *)karg)->dest_chn, &((sys_bind_args *)karg)->src_chn);
		break;
	case SYS_GETBINDBYSRC:
		ret = sys_get_bind_by_src(&((sys_bind_src_args *)karg)->src_chn, &((sys_bind_src_args *)karg)->dest_chns);
		break;
	case SYS_GET_CUST_CODE:
		ret = sys_drv_get_custom_code((hi_u32 *)karg);
		break;
	case SYS_SET_TIME_ZONE:
		if (karg == HI_NULL) {
			ret = HI_ERR_SYS_NULL_PTR;
			break;
		}
		if ((*(hi_s32 *)karg < -86400) || (*(hi_s32 *)karg > 86400)) {
			ret = HI_ERR_SYS_ILLEGAL_PARAM;
			break;
		}
		g_sys_time_zone = *(hi_s32 *)karg;
		ret = HI_SUCCESS;
		break;
	case SYS_GET_TIME_ZONE:
		ret = sys_get_time_zone((hi_s32 *)karg);
		break;
	case SYS_SET_GPS_INFO:
		ret = sys_copy_to_arg(&g_gps_info, karg, sizeof(g_gps_info));
		break;
	case SYS_GET_GPS_INFO:
		ret = sys_copy_to_arg(karg, &g_gps_info, sizeof(g_gps_info));
		break;
	case SYS_SET_CSC_MATRIX:
		if (karg == HI_NULL) {
			ret = HI_ERR_SYS_NULL_PTR;
			break;
		}
		osal_spin_lock(&g_sys_func_spin_lock);
		if (((hi_csc_param *)karg)->mode == HI_ID_VDEC) {
			ret = (g_dvpp_set_csc_param.set_vdec_csc_param == HI_NULL) ?
				HI_ERR_SYS_NULL_PTR : g_dvpp_set_csc_param.set_vdec_csc_param((hi_csc_param *)karg);
		} else if (((hi_csc_param *)karg)->mode == HI_ID_VPC) {
			ret = (g_dvpp_set_csc_param.set_vpc_csc_param == HI_NULL) ?
				HI_ERR_SYS_NULL_PTR : g_dvpp_set_csc_param.set_vpc_csc_param((hi_csc_param *)karg);
		} else {
			ret = HI_ERR_SYS_UNEXIST;
		}
		osal_spin_unlock(&g_sys_func_spin_lock);
		break;
	case SYS_GET_CSC_MATRIX:
		if (karg == HI_NULL) {
			ret = HI_ERR_SYS_NULL_PTR;
			break;
		}
		osal_spin_lock(&g_sys_func_spin_lock);
		if (((hi_csc_param *)karg)->mode == HI_ID_VDEC) {
			ret = (g_dvpp_get_csc_param.get_vdec_csc_param == HI_NULL) ?
				HI_ERR_SYS_NULL_PTR : g_dvpp_get_csc_param.get_vdec_csc_param((hi_csc_param *)karg);
		} else if (((hi_csc_param *)karg)->mode == HI_ID_VPC) {
			ret = (g_dvpp_get_csc_param.get_vpc_csc_param == HI_NULL) ?
				HI_ERR_SYS_NULL_PTR : g_dvpp_get_csc_param.get_vpc_csc_param((hi_csc_param *)karg);
		} else {
			ret = HI_ERR_SYS_UNEXIST;
		}
		osal_spin_unlock(&g_sys_func_spin_lock);
		break;
	case SYS_GET_CHIP_TYPE:
		if (karg == HI_NULL) {
			ret = HI_ERR_SYS_NULL_PTR;
			break;
		}
		*(hi_u32 *)karg = g_sys_chip_type;
		ret = HI_SUCCESS;
		break;
	case SYS_GET_VPSSVENC_WRAP_BUF_LINE:
		ret = sys_drv_get_vpss_venc_wrap_buffer_line(HI_NULL, HI_NULL);
		break;
	case SYS_SET_RAW_FRAME_COMPRESS_RATE:
		ret = sys_drv_set_raw_frame_compress((hi_raw_frame_compress_param *)karg);
		break;
	case SYS_GET_RAW_FRAME_COMPRESS_RATE:
		ret = sys_drv_get_raw_frame_compress((hi_raw_frame_compress_param *)karg);
		break;
	default:
		ret = HI_FAILURE;
		break;
	}

	osal_atomic_dec_return(&g_sys_busy_cnt);
	osal_up(&g_sys_ctx);
	return ret;
}

static hi_slong sys_compat_ioctl(hi_u32 cmd, hi_ulong arg, hi_void *private_data)
{
	return sys_ioctl(cmd, arg, private_data);
}

static hi_s32 sys_freeze(const osal_dev_t *dev)
{
	(void)dev;
	g_sys_state = SYS_RUNTIME_BLOCKED;
	return HI_SUCCESS;
}

static hi_s32 sys_restore(const osal_dev_t *dev)
{
	(void)dev;
	g_sys_state = g_runtime_state;
	return HI_SUCCESS;
}

int sys_do_mod_init(hi_void)
{
	hi_s32 ret;
	osal_task_t *pg_task = HI_NULL;

	osal_asm_get_clock();
	g_svm_check_mem_attribute = osal_symbol_get("hal_kernel_svm_check_mem_attribute");
	g_drval_devdrv_get_devinfo = osal_symbol_get("drval_devdrv_get_devinfo");
	g_drval_get_pg_info = osal_symbol_get("drval_get_pg_info");
	g_drval_check_pg_info_support = osal_symbol_get("drval_check_pg_info_support");

	sys_drv_read_chip_info();
	sys_init_core_usable_info();

	ret = osal_spin_lock_init(&g_sys_spin_lock);
	if (ret != HI_SUCCESS)
		return ret;
	ret = osal_atomic_init(&g_sys_busy_cnt);
	if (ret != HI_SUCCESS)
		goto err_spin;
	ret = osal_atomic_init(&g_sys_open_cnt);
	if (ret != HI_SUCCESS)
		goto err_busy_atomic;
	ret = osal_sema_init(&g_sys_ctx, 1);
	if (ret != HI_SUCCESS)
		goto err_open_atomic;
	ret = osal_spin_lock_init(&g_sys_func_spin_lock);
	if (ret != HI_SUCCESS)
		goto err_ctx;

	osal_init_completion(&pg_info_event);
	if (g_drval_get_pg_info != HI_NULL || g_drval_check_pg_info_support != HI_NULL) {
		pg_task = osal_kthread_create(sys_pg_info_thread, HI_NULL, "sys_get_pg_info");
		if (pg_task != HI_NULL) {
			osal_wait_for_completion(&pg_info_event);
			osal_kthread_destory(pg_task, 1);
		}
	}

	g_runtime_state = SYS_RUNTIME_IDLE;
	g_sys_state = SYS_RUNTIME_IDLE;
	g_conf = HI_FALSE;
	g_bootdot_init_ok = HI_FALSE;
	memset(&g_sys_config, 0, sizeof(g_sys_config));
	memset(&g_gps_info, 0, sizeof(g_gps_info));
	memset(&g_raw_frame_compress, 0, sizeof(g_raw_frame_compress));
	memset(g_mem_ctx, 0, sizeof(g_mem_ctx));
	memset(g_dvpp_vf_bitmap, 0, sizeof(g_dvpp_vf_bitmap));
	memset(g_irq_cpumask, 0, sizeof(g_irq_cpumask));

	g_sys_device = osal_createdev("sys");
	if (g_sys_device == HI_NULL) {
		ret = HI_FAILURE;
		goto err_func_spin;
	}
	g_sys_device->minor = SYS_DEVICE_MINOR;
	g_sys_device->fops = &g_sys_fops;
	g_sys_device->osal_pmops = &g_sys_pmops;

	ret = osal_registerdevice(g_sys_device);
	if (ret != HI_SUCCESS)
		goto err_dev;

	ret = cmpi_register_module(&g_sys_module);
	if (ret != HI_SUCCESS)
		goto err_register_dev;

	g_sys_proc = osal_create_proc_entry("sys", HI_NULL);
	if (g_sys_proc != HI_NULL)
		g_sys_proc->read = sys_proc_show_ex;

	ret = sys_bind_mod_init();
	if (ret != HI_SUCCESS)
		goto err_module;
	sys_bind_init();

	ret = osal_sema_init(&g_sys_sem, 1);
	if (ret != HI_SUCCESS)
		goto err_bind;

	sys_init_ip_num();
	return HI_SUCCESS;

err_bind:
	sys_bind_exit();
	sys_bind_mod_exit();
err_module:
	if (g_sys_proc != HI_NULL) {
		osal_remove_proc_entry("sys", HI_NULL);
		g_sys_proc = HI_NULL;
	}
	cmpi_unregister_module(HI_ID_SYS);
err_register_dev:
	osal_deregisterdevice(g_sys_device);
err_dev:
	osal_destroydev(g_sys_device);
	g_sys_device = HI_NULL;
err_func_spin:
	osal_spin_lock_destory(&g_sys_func_spin_lock);
err_ctx:
	osal_sema_destory(&g_sys_ctx);
err_open_atomic:
	osal_atomic_destory(&g_sys_open_cnt);
err_busy_atomic:
	osal_atomic_destory(&g_sys_busy_cnt);
err_spin:
	osal_spin_lock_destory(&g_sys_spin_lock);
	if (g_drval_check_pg_info_support != HI_NULL)
		osal_symbol_put("drval_check_pg_info_support");
	if (g_drval_get_pg_info != HI_NULL)
		osal_symbol_put("drval_get_pg_info");
	if (g_drval_devdrv_get_devinfo != HI_NULL)
		osal_symbol_put("drval_devdrv_get_devinfo");
	if (g_svm_check_mem_attribute != HI_NULL)
		osal_symbol_put("hal_kernel_svm_check_mem_attribute");
	return ret;
}

void sys_do_mod_exit(hi_void)
{
	(void)sys_drv_exit();
	osal_sema_destory(&g_sys_sem);
	sys_bind_exit();
	sys_bind_mod_exit();
	sys_exit_mem_conf();
	if (g_sys_proc != HI_NULL) {
		osal_remove_proc_entry("sys", HI_NULL);
		g_sys_proc = HI_NULL;
	}
	cmpi_unregister_module(HI_ID_SYS);
	if (g_sys_device != HI_NULL) {
		osal_deregisterdevice(g_sys_device);
		osal_destroydev(g_sys_device);
		g_sys_device = HI_NULL;
	}
	osal_spin_lock_destory(&g_sys_func_spin_lock);
	osal_sema_destory(&g_sys_ctx);
	osal_atomic_destory(&g_sys_open_cnt);
	osal_atomic_destory(&g_sys_busy_cnt);
	osal_spin_lock_destory(&g_sys_spin_lock);
	if (g_drval_check_pg_info_support != HI_NULL)
		osal_symbol_put("drval_check_pg_info_support");
	if (g_drval_get_pg_info != HI_NULL)
		osal_symbol_put("drval_get_pg_info");
	if (g_drval_devdrv_get_devinfo != HI_NULL)
		osal_symbol_put("drval_devdrv_get_devinfo");
	if (g_svm_check_mem_attribute != HI_NULL)
		osal_symbol_put("hal_kernel_svm_check_mem_attribute");
}

module_init(sys_do_mod_init);
module_exit(sys_do_mod_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Reconstructed drv_sys based on IDA MCP");
