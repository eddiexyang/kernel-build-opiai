// SPDX-License-Identifier: GPL-2.0
#include <linux/cred.h>
#include <linux/dcache.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/path.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/uidgid.h>
#include <linux/workqueue.h>

#include "hiss/hsm_status.h"
#include "toolchain/bbox/device/bbox_pub.h"

#define HSM_HEARTBEAT_INVALID_RET      1
#define HSM_TEEK_SYMBOL_ERR            0xFFFF0007U

#define HSM_TEEK_LOGIN_IDENTIFY        7U
#define HSM_OPEN_SESSION_PARAM_55      0x55U
#define HSM_RESET_ACTION_PARAM_1       0x1U
#define HSM_ACCELERATOR_CHECK_PARAM_13 0x13U
#define HSM_READ_PG_INFO_PARAM_711     0x711U
#define HSM_HISS_STATUS_PARAM_1112     0x1112U

#define HSM_CMD_NOTIFY_RESET_ACTION    0x5000U
#define HSM_CMD_ACCELERATOR_CHECK      0x5001U
#define HSM_CMD_READ_PG_INFO           0x6000U
#define HSM_CMD_GET_HISS_STATUS        0x331CU

#define HSM_PATH_MAX                   260U
#define HSM_MAX_DEV_ID                 1U
#define HSM_MAX_PG_BUF_SIZE            0x1000U

#define HSM_MMIO_BASE                  0x23e60000UL
#define HSM_MMIO_SIZE                  0x40000U

#define HSM_INIT_DELAY_MS              1000U
#define HSM_PERIODIC_DELAY_MS          60000U

#define HSM_EXCEPTION_NOT_INITIALIZED  0xA6360000U
#define HSM_EXCEPTION_INACTIVE         0xA6361000U
#define HSM_EXCEPTION_ABNORMAL         0xA6362000U

struct hsm_teek_context {
	u8 opaque[32];
};

struct hsm_teek_session {
	u8 opaque[32];
};

struct hsm_teek_value {
	u32 a;
	u32 b;
	u64 reserved;
};

struct hsm_teek_tmpref {
	void *buffer;
	u64 size;
};

union hsm_teek_param {
	struct hsm_teek_value value;
	struct hsm_teek_tmpref tmpref;
};

struct hsm_teek_operation {
	u32 started;
	u32 param_types;
	union hsm_teek_param params[4];
	u64 session;
	u64 cancel_flag;
};

struct hsm_open_details {
	u32 uid;
	char path[HSM_PATH_MAX];
};

struct hsm_source_data {
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct workqueue_struct *workqueue;
	struct work_struct work;
	struct timer_list timer;
	void __iomem *status_base;
	u32 exception_state;
	u32 failure_count;
	u32 stop;
	u8 dev_id;
	bool session_ready;
	bool has_last_timestamp;
};

typedef u32 (*hsm_teek_init_fn)(const char *name, struct hsm_teek_context *ctx);
typedef void (*hsm_teek_fini_fn)(struct hsm_teek_context *ctx);
typedef u32 (*hsm_teek_open_fn)(struct hsm_teek_context *ctx,
	struct hsm_teek_session *session, const void *uuid, u32 login_method,
	void *connection_data, struct hsm_teek_operation *operation,
	u32 *return_origin);
typedef void (*hsm_teek_close_fn)(struct hsm_teek_session *session);
typedef u32 (*hsm_teek_invoke_fn)(struct hsm_teek_session *session, u32 command,
	struct hsm_teek_operation *operation, u32 *return_origin);

static hsm_teek_init_fn g_teek_init;
static hsm_teek_fini_fn g_teek_fini;
static hsm_teek_open_fn g_teek_open;
static hsm_teek_close_fn g_teek_close;
static hsm_teek_invoke_fn g_teek_invoke;
static struct hsm_source_data *g_hd;
static DEFINE_MUTEX(g_hd_lock);

static const u8 g_tee_uuid[16] = {
	0x21, 0x0a, 0x42, 0x9d, 0x40, 0xb4, 0x3e, 0x47,
	0xb3, 0x54, 0xab, 0x93, 0x10, 0xe2, 0xa6, 0xd1,
};

static void *hsm_heartbeat_symbol_get(const char *name)
{
	void *sym = __symbol_get(name);

	if (!sym)
		pr_err("3[HSM HEARTBEAT]: get symbol %s failed.\n", name);

	return sym;
}

static u32 hsm_teek_initialize_ctx(const char *name, struct hsm_teek_context *ctx)
{
	if (!g_teek_init)
		g_teek_init = (hsm_teek_init_fn)hsm_heartbeat_symbol_get("TEEK_InitializeContext");
	if (!g_teek_init)
		return HSM_TEEK_SYMBOL_ERR;

	return g_teek_init(name, ctx);
}

static void hsm_teek_finalize_ctx(struct hsm_teek_context *ctx)
{
	if (!g_teek_fini)
		g_teek_fini = (hsm_teek_fini_fn)hsm_heartbeat_symbol_get("TEEK_FinalizeContext");
	if (g_teek_fini)
		g_teek_fini(ctx);
}

static u32 hsm_teek_open_session(struct hsm_teek_context *ctx,
	struct hsm_teek_session *session, const void *uuid, u32 login_method,
	void *connection_data, struct hsm_teek_operation *operation,
	u32 *origin)
{
	if (!g_teek_open)
		g_teek_open = (hsm_teek_open_fn)hsm_heartbeat_symbol_get("TEEK_OpenSession");
	if (!g_teek_open)
		return HSM_TEEK_SYMBOL_ERR;

	return g_teek_open(ctx, session, uuid, login_method, connection_data,
		operation, origin);
}

static void hsm_teek_close_session(struct hsm_teek_session *session)
{
	if (!g_teek_close)
		g_teek_close = (hsm_teek_close_fn)hsm_heartbeat_symbol_get("TEEK_CloseSession");
	if (g_teek_close)
		g_teek_close(session);
}

static u32 hsm_teek_invoke_command(struct hsm_teek_session *session, u32 command,
	struct hsm_teek_operation *operation, u32 *origin)
{
	if (!g_teek_invoke)
		g_teek_invoke = (hsm_teek_invoke_fn)hsm_heartbeat_symbol_get("TEEK_InvokeCommand");
	if (!g_teek_invoke)
		return HSM_TEEK_SYMBOL_ERR;

	return g_teek_invoke(session, command, operation, origin);
}

static struct hsm_source_data *get_hsm_source_data(void)
{
	struct hsm_source_data *hd = g_hd;

	if (hd)
		return hd;

	mutex_lock(&g_hd_lock);
	if (!g_hd) {
		g_hd = kzalloc(sizeof(*g_hd), GFP_KERNEL);
		if (!g_hd)
			pr_err("3[HSM HEARTBEAT]: alloc hsm source data failed.\n");
	}
	hd = g_hd;
	mutex_unlock(&g_hd_lock);

	return hd;
}

static int hsm_get_process_details(struct hsm_open_details *details)
{
	struct file *exe_file = NULL;
	char *path_buf = NULL;
	char *path = NULL;
	const struct cred *cred = current_cred();

	if (!details)
		return -EINVAL;

	memset(details, 0, sizeof(*details));
	details->uid = __kuid_val(cred->uid);

	if (current->flags & PF_KTHREAD)
		return strscpy(details->path, "kernel_thread", sizeof(details->path)) < 0 ? -EINVAL : 0;

	if (current->mm && current->mm->exe_file) {
		exe_file = current->mm->exe_file;
		get_file(exe_file);
	}

	if (!exe_file) {
		if (strscpy(details->path, current->comm, sizeof(details->path)) < 0)
			return -EINVAL;
		return 0;
	}

	path_buf = kmalloc(PATH_MAX, GFP_KERNEL);
	if (!path_buf) {
		fput(exe_file);
		return -ENOMEM;
	}

	path = d_path(&exe_file->f_path, path_buf, PATH_MAX);
	if (IS_ERR(path)) {
		kfree(path_buf);
		fput(exe_file);
		return PTR_ERR(path);
	}

	if (strscpy(details->path, path, sizeof(details->path)) < 0) {
		kfree(path_buf);
		fput(exe_file);
		return -EINVAL;
	}

	kfree(path_buf);
	fput(exe_file);
	return 0;
}

static int hsm_heartbeat_open_session(u32 dev_id, struct hsm_teek_context *ctx,
	struct hsm_teek_session *session)
{
	struct hsm_open_details details;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;
	size_t path_len;
	int detail_ret;

	if (dev_id > HSM_MAX_DEV_ID)
		return HSM_HEARTBEAT_INVALID_RET;

	ret = hsm_teek_initialize_ctx(NULL, ctx);
	if (ret)
		return (int)ret;

	detail_ret = hsm_get_process_details(&details);
	if (detail_ret) {
		hsm_teek_finalize_ctx(ctx);
		return detail_ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_OPEN_SESSION_PARAM_55;
	op.params[0].tmpref.buffer = &details.uid;
	op.params[0].tmpref.size = sizeof(details.uid);

	path_len = strnlen(details.path, sizeof(details.path));
	if (path_len >= sizeof(details.path)) {
		hsm_teek_finalize_ctx(ctx);
		return -EINVAL;
	}
	op.params[1].tmpref.buffer = details.path;
	op.params[1].tmpref.size = path_len + 1;

	ret = hsm_teek_open_session(ctx, session, g_tee_uuid, HSM_TEEK_LOGIN_IDENTIFY,
		NULL, &op, &origin);
	if (ret)
		hsm_teek_finalize_ctx(ctx);

	return (int)ret;
}

static void hsm_report_exception(u32 exception_id)
{
	struct hsm_source_data *hd = get_hsm_source_data();
	struct bbox_report_info info;
	struct timespec64 ts;
	int ret;

	if (!hd)
		return;

	memset(&info, 0, sizeof(info));
	ktime_get_real_ts64(&ts);
	info.devid = hd->dev_id;
	info.excepid = exception_id;
	info.time.tv_sec = ts.tv_sec;
	info.time.tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	info.arg = exception_id;

	ret = bbox_exception_report(&info);
	if (ret)
		pr_err("3[HSM HEARTBEAT]: bbox_exception_report failed, id=0x%x ret=%d.\n",
			exception_id, ret);
}

static int hsm_invoke_state_check(struct hsm_source_data *hd)
{
	struct hsm_teek_operation op;
	struct timespec64 ts;
	u32 usec;
	u32 origin = 0;
	u32 ret;

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_HISS_STATUS_PARAM_1112;

	ktime_get_real_ts64(&ts);
	if (hd->has_last_timestamp) {
		usec = (u32)(ts.tv_nsec / NSEC_PER_USEC);
		op.params[1].value.a = lower_32_bits(ts.tv_sec);
		op.params[1].value.b = upper_32_bits(ts.tv_sec);
		op.params[2].value.a = usec;
		op.params[2].value.b = 0;
	} else {
		hd->has_last_timestamp = true;
	}

	ret = hsm_teek_invoke_command(&hd->session, HSM_CMD_GET_HISS_STATUS, &op, &origin);
	if (ret) {
		if ((hd->failure_count % 1000U) == 0)
			pr_err("3[HSM HEARTBEAT]: heartbeat invoke failed, ret=0x%x origin=0x%x.\n",
				ret, origin);
		hd->failure_count++;
		if (hd->exception_state == 0)
			hsm_report_exception(HSM_EXCEPTION_NOT_INITIALIZED);
		else if (hd->exception_state == 1)
			hsm_report_exception(HSM_EXCEPTION_INACTIVE);
		hd->exception_state = 2;
		return (int)ret;
	}

	if ((op.params[0].value.a & 0xFFFFFF7FU) == 0x7FU) {
		if (hd->exception_state == 0)
			hd->exception_state = 1;
		return 0;
	}

	if ((hd->failure_count % 1000U) == 0)
		pr_err("3[HSM HEARTBEAT]: heartbeat returned abnormal state 0x%x.\n",
			op.params[0].value.a);
	hd->failure_count++;
	if (hd->exception_state <= 1U) {
		hsm_report_exception(HSM_EXCEPTION_ABNORMAL);
		hd->exception_state = 2;
	}

	return 0;
}

static void hsm_timeout_work(struct work_struct *work)
{
	struct hsm_source_data *hd = container_of(work, struct hsm_source_data, work);
	u32 ret;

	if (hd->session_ready) {
		hsm_invoke_state_check(hd);
		return;
	}

	ret = hsm_heartbeat_open_session(hd->dev_id, &hd->ctx, &hd->session);
	if (ret) {
		if ((hd->failure_count % 1000U) == 0)
			pr_err("3[HSM HEARTBEAT]: open session for timeout work failed, ret=0x%x.\n",
				ret);
		hd->failure_count++;
		return;
	}

	hd->session_ready = true;
	hsm_invoke_state_check(hd);
}

static void hsm_timer_callback(struct timer_list *timer)
{
	struct hsm_source_data *hd = from_timer(hd, timer, timer);

	if (READ_ONCE(hd->stop))
		return;

	queue_work(system_wq, &hd->work);
	mod_timer(&hd->timer, jiffies + msecs_to_jiffies(HSM_PERIODIC_DELAY_MS));
}

static int read_pg_info_common(u32 dev_id, pg_cmd_data cmd, void *out_buf, u32 out_size)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	ret = hsm_heartbeat_open_session(dev_id, &ctx, &session);
	if (ret)
		return (int)ret;

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_READ_PG_INFO_PARAM_711;
	op.params[0].value.a = dev_id;
	op.params[1].value.a = (u32)cmd.module;
	op.params[1].value.b = (u32)cmd.data;
	op.params[2].tmpref.buffer = out_buf;
	op.params[2].tmpref.size = out_size;

	ret = hsm_teek_invoke_command(&session, HSM_CMD_READ_PG_INFO, &op, &origin);
	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);

	return (int)ret;
}

int hsm_notify_reset_action(u32 dev_id)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	ret = hsm_heartbeat_open_session(dev_id, &ctx, &session);
	if (ret)
		return (int)ret;

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_RESET_ACTION_PARAM_1;
	op.params[0].value.a = dev_id;

	ret = hsm_teek_invoke_command(&session, HSM_CMD_NOTIFY_RESET_ACTION, &op, &origin);
	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);

	return (int)ret;
}
EXPORT_SYMBOL(hsm_notify_reset_action);

int hsm_accelerator_check(u32 dev_id, u32 *hsm_accelerator_status, u32 hsm_status_len)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	if (!hsm_accelerator_status || hsm_status_len != sizeof(*hsm_accelerator_status))
		return HSM_HEARTBEAT_INVALID_RET;

	ret = hsm_heartbeat_open_session(dev_id, &ctx, &session);
	if (ret)
		return (int)ret;

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_ACCELERATOR_CHECK_PARAM_13;
	op.params[0].value.b = sizeof(*hsm_accelerator_status);
	op.params[1].value.a = dev_id;

	ret = hsm_teek_invoke_command(&session, HSM_CMD_ACCELERATOR_CHECK, &op, &origin);
	if (!ret)
		*hsm_accelerator_status = op.params[0].value.a;

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(hsm_accelerator_check);

int hsm_get_hiss_status(u32 dev_id, hsm_status_t *hsm_status)
{
	struct hsm_source_data *hd = get_hsm_source_data();
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	if (!hd)
		return -ENOMEM;
	if (!hsm_status)
		return HSM_HEARTBEAT_INVALID_RET;

	ret = hsm_heartbeat_open_session(dev_id, &ctx, &session);
	if (ret)
		return (int)ret;

	memset(hsm_status, 0, sizeof(*hsm_status));
	hsm_status->hsm_status_code = HSM_OK;

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_HISS_STATUS_PARAM_1112;
	op.params[3].value.a = dev_id;

	ret = hsm_teek_invoke_command(&session, HSM_CMD_GET_HISS_STATUS, &op, &origin);
	if (ret) {
		hsm_status->hsm_status_code = hd->exception_state ? HSM_ERROR : HSM_NOT_INITIALIZED;
	} else {
		hsm_status->hsm_status_info = op.params[0].value.a;
	}

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(hsm_get_hiss_status);

int sec_read_pg_info(u32 dev_id, pg_cmd_data cmd, u64 *out_arg)
{
	if (!out_arg)
		return HSM_HEARTBEAT_INVALID_RET;

	return read_pg_info_common(dev_id, cmd, out_arg, sizeof(*out_arg));
}
EXPORT_SYMBOL(sec_read_pg_info);

int sec_read_pg_info_block(u32 dev_id, pg_cmd_data cmd, u8 *buf, u32 buf_len)
{
	if (!buf || buf_len == 0 || buf_len > HSM_MAX_PG_BUF_SIZE)
		return HSM_HEARTBEAT_INVALID_RET;

	return read_pg_info_common(dev_id, cmd, buf, buf_len);
}
EXPORT_SYMBOL(sec_read_pg_info_block);

static int __init hsm_heart_beat_init(void)
{
	struct hsm_source_data *hd = get_hsm_source_data();

	if (!hd)
		return -ENOMEM;

	hd->workqueue = alloc_workqueue("hsm_heartbeat", WQ_UNBOUND | WQ_MEM_RECLAIM, 1);
	if (!hd->workqueue) {
		kfree(hd);
		g_hd = NULL;
		return -ENOMEM;
	}

	hd->status_base = ioremap(HSM_MMIO_BASE, HSM_MMIO_SIZE);
	if (!hd->status_base) {
		destroy_workqueue(hd->workqueue);
		hd->workqueue = NULL;
		kfree(hd);
		g_hd = NULL;
		return -ENOMEM;
	}

	hd->dev_id = 0;
	hd->session_ready = false;
	hd->has_last_timestamp = false;
	INIT_WORK(&hd->work, hsm_timeout_work);
	timer_setup(&hd->timer, hsm_timer_callback, 0);
	mod_timer(&hd->timer, jiffies + msecs_to_jiffies(HSM_INIT_DELAY_MS));

	return 0;
}

static void __exit hsm_heart_beat_exit(void)
{
	struct hsm_source_data *hd = g_hd;

	if (!hd)
		return;

	WRITE_ONCE(hd->stop, 1);
	del_timer_sync(&hd->timer);
	cancel_work_sync(&hd->work);
	if (hd->workqueue) {
		flush_workqueue(hd->workqueue);
		destroy_workqueue(hd->workqueue);
	}
	if (hd->session_ready)
		hsm_teek_close_session(&hd->session);
	hsm_teek_finalize_ctx(&hd->ctx);
	if (hd->status_base)
		iounmap(hd->status_base);

	mutex_lock(&g_hd_lock);
	g_hd = NULL;
	mutex_unlock(&g_hd_lock);
	kfree(hd);
}

module_init(hsm_heart_beat_init);
module_exit(hsm_heart_beat_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("HSM HEARTBEAT CHECK");
MODULE_VERSION("V1.0");
