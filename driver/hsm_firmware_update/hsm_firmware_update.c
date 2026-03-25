// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/cred.h>
#include <linux/uidgid.h>
#include <linux/file.h>
#include <linux/path.h>
#include <linux/sched.h>

#include "securec.h"
#include "hsm_fw_update.h"

#define HSM_FW_INVALID_RET          1
#define HSM_TEEK_SYMBOL_ERR         0xFFFF0007U
#define HSM_FW_COPY_ERR             (-65521)
#define HSM_FW_NOMEM_ERR            (-65524)

#define HSM_OPEN_SESSION_PARAM_55   0x55U
#define HSM_PARAM_11                0x11U
#define HSM_PARAM_16                0x16U
#define HSM_PARAM_21                0x21U
#define HSM_PARAM_51                0x51U
#define HSM_PARAM_111               0x111U
#define HSM_PARAM_611               0x611U

#define HSM_TEEK_LOGIN_IDENTIFY     7U
#define HSM_FW_MAX_DEV_ID           1U
#define HSM_FW_MAX_IMG_NUM          9U
#define HSM_FW_MAX_IMG_SIZE         0x400000U
#define HSM_FW_RIM_LEN              0x220U
#define HSM_FW_VERSION_LEN          16U
#define HSM_FW_ALLOC_RETRY          10U
#define HSM_FW_NV_CNT_SIZE          sizeof(u32)
#define HSM_FW_PATH_MAX             260U
#define HSM_FW_RESETCNT_TIMEOUT     2500L

#define HSM_FW_CMD_IMG_VERIFY                 0x7000U
#define HSM_FW_CMD_IMG_UPDATE                 0x7001U
#define HSM_FW_CMD_CLEAR_FLASH_RESETCNT       0x7102U
#define HSM_FW_CMD_SYNC_BEFORE_UPDATE         0x7103U
#define HSM_FW_CMD_FLASH_READ_CMDLINE         0x7104U
#define HSM_FW_CMD_SOC_GET_NVCNT             0x7105U
#define HSM_FW_CMD_RESET_RECOVERY_BOOT_COUNT 0x7106U
#define HSM_FW_CMD_UFSCK_KEY_CFG             0x7107U
#define HSM_FW_CMD_READ_NSFORBID             0x7108U
#define HSM_FW_CMD_GET_FW_VERIFY_RESULT      0x7109U
#define HSM_FW_CMD_UPDATE_FINISH             0x7003U
#define HSM_FW_CMD_SYNC_AND_EFUSE_UPDATE     0x7004U
#define HSM_FW_CMD_RIM_UPDATE                0x7005U
#define HSM_FW_CMD_FLASH_GET_VERSION         0x700AU
#define HSM_FW_CMD_FLASH_GET_COUNT           0x700BU
#define HSM_FW_CMD_FLASH_GET_INFO            0x700CU

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

struct hsm_fw_open_details {
	u32 uid;
	char path[HSM_FW_PATH_MAX];
};

struct hsm_resetcnt_work {
	struct work_struct work;
	struct completion done;
	int result;
	u32 dev_id;
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

static const u8 g_tee_uuid[16] = {
	0xd8, 0x9c, 0x4c, 0x74, 0xec, 0x5a, 0x0c, 0x45,
	0xa9, 0xbc, 0x73, 0x31, 0x89, 0xd3, 0xb3, 0xa0,
};

static DEFINE_MUTEX(g_hsm_work_mutex);
static struct hsm_resetcnt_work g_hsm_work;
static struct workqueue_struct *g_workqueue;
static void *g_img_buf[HSM_FW_MAX_DEV_ID + 1];
static DEFINE_MUTEX(g_upgrade_img0);
static DEFINE_MUTEX(g_upgrade_img1);
static struct mutex * const g_upgrade_img[HSM_FW_MAX_DEV_ID + 1] = {
	&g_upgrade_img0,
	&g_upgrade_img1,
};

static void *hsm_fw_symbol_get(const char *name)
{
	void *sym = __symbol_get(name);

	if (!sym)
		pr_err("3[HSM_AUTH]: get symbol %s failed.\n", name);

	return sym;
}

static u32 hsm_teek_initialize_ctx(struct hsm_teek_context *ctx)
{
	if (!g_teek_init)
		g_teek_init = (hsm_teek_init_fn)hsm_fw_symbol_get("TEEK_InitializeContext");
	if (!g_teek_init)
		return HSM_TEEK_SYMBOL_ERR;

	return g_teek_init(NULL, ctx);
}

static void hsm_teek_finalize_ctx(struct hsm_teek_context *ctx)
{
	if (!g_teek_fini)
		g_teek_fini = (hsm_teek_fini_fn)hsm_fw_symbol_get("TEEK_FinalizeContext");
	if (g_teek_fini)
		g_teek_fini(ctx);
}

static u32 hsm_teek_open_session(struct hsm_teek_context *ctx,
	struct hsm_teek_session *session, const void *uuid,
	struct hsm_teek_operation *operation, u32 *origin)
{
	if (!g_teek_open)
		g_teek_open = (hsm_teek_open_fn)hsm_fw_symbol_get("TEEK_OpenSession");
	if (!g_teek_open)
		return HSM_TEEK_SYMBOL_ERR;

	return g_teek_open(ctx, session, uuid, HSM_TEEK_LOGIN_IDENTIFY, NULL, operation, origin);
}

static void hsm_teek_close_session(struct hsm_teek_session *session)
{
	if (!g_teek_close)
		g_teek_close = (hsm_teek_close_fn)hsm_fw_symbol_get("TEEK_CloseSession");
	if (g_teek_close)
		g_teek_close(session);
}

static u32 hsm_teek_invoke_command(struct hsm_teek_session *session, u32 command,
	struct hsm_teek_operation *operation, u32 *origin)
{
	if (!g_teek_invoke)
		g_teek_invoke = (hsm_teek_invoke_fn)hsm_fw_symbol_get("TEEK_InvokeCommand");
	if (!g_teek_invoke)
		return HSM_TEEK_SYMBOL_ERR;

	return g_teek_invoke(session, command, operation, origin);
}

static int device_id_check(u32 dev_id)
{
	return dev_id > HSM_FW_MAX_DEV_ID;
}

static int hsm_get_process_details(struct hsm_fw_open_details *details)
{
	struct file *exe_file = NULL;
	char *path_buf = NULL;
	char *path = NULL;
	const struct cred *cred = current_cred();

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

static int firmware_open_session(u32 dev_id, struct hsm_teek_context *ctx,
	struct hsm_teek_session *session)
{
	struct hsm_fw_open_details details;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;
	size_t path_len;
	int detail_ret;

	if (device_id_check(dev_id)) {
		pr_err("3[HSM firmware]:[%d] invalid dev id, 0x%x.\n", __LINE__, dev_id);
		return HSM_FW_INVALID_RET;
	}

	ret = hsm_teek_initialize_ctx(ctx);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] TEEK initialize context failed, 0x%x.\n",
			__LINE__, ret);
		return (int)ret;
	}

	detail_ret = hsm_get_process_details(&details);
	if (detail_ret) {
		pr_err("3[HSM firmware]:[%d] TEEK get process detail failed, 0x%x.\n",
			__LINE__, detail_ret);
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

	ret = hsm_teek_open_session(ctx, session, g_tee_uuid, &op, &origin);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] TEEK open session failed, 0x%x, 0x%x, %s.\n",
			__LINE__, ret, origin, details.path);
		hsm_teek_finalize_ctx(ctx);
	}

	return (int)ret;
}

static int sec_img_sync_before_update(u32 dev_id, struct hsm_teek_session *session)
{
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_PARAM_11;
	op.params[0].value.a = dev_id;

	ret = hsm_teek_invoke_command(session, HSM_FW_CMD_SYNC_BEFORE_UPDATE, &op, &origin);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] sec img sync before update failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);
		return (int)ret;
	}

	return 0;
}

static int hsm_fw_alloc_img_buf(u32 dev_id)
{
	u32 i;

	for (i = 0; i < HSM_FW_ALLOC_RETRY; i++) {
		g_img_buf[dev_id] = kmalloc(HSM_FW_MAX_IMG_SIZE, GFP_KERNEL | __GFP_NOWARN);
		if (g_img_buf[dev_id])
			return 0;
	}

	pr_err("3[HSM firmware]:[%d] malloc buf failed.\n", __LINE__);
	return HSM_FW_NOMEM_ERR;
}

static void clear_flash_resetcnt_wq(struct work_struct *work)
{
	struct hsm_resetcnt_work *reset_work =
		container_of(work, struct hsm_resetcnt_work, work);
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	reset_work->result = HSM_FW_INVALID_RET;
	memset(&op, 0, sizeof(op));

	ret = firmware_open_session(reset_work->dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] clear flash resetcnt failed, 0x%x.\n",
			__LINE__, ret);
		complete(&reset_work->done);
		return;
	}

	op.started = 1;
	op.param_types = HSM_PARAM_11;
	op.params[0].value.a = reset_work->dev_id;

	ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_CLEAR_FLASH_RESETCNT, &op, &origin);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] resetcnt clear failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);
	} else {
		reset_work->result = 0;
		pr_info("6[HSM firmware]:[%d] clear resetcnt success.\n", __LINE__);
	}

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	complete(&reset_work->done);
}

int clear_flash_resetcnt(u32 dev_id)
{
	int ret;
	long timeout;

	mutex_lock(&g_hsm_work_mutex);

	g_hsm_work.result = 0;
	g_hsm_work.dev_id = dev_id;
	reinit_completion(&g_hsm_work.done);
	queue_work_on(WORK_CPU_UNBOUND, g_workqueue, &g_hsm_work.work);

	timeout = wait_for_completion_killable_timeout(&g_hsm_work.done, HSM_FW_RESETCNT_TIMEOUT);
	if (!timeout) {
		pr_err("3[HSM firmware]:[%d] completion wait timeout.\n", __LINE__);
		ret = -1;
		mutex_unlock(&g_hsm_work_mutex);
		return ret;
	}

	ret = g_hsm_work.result;
	if (ret)
		pr_err("3[HSM firmware]:[%d] clear flash resetcnt failed, 0x%x.\n",
			__LINE__, ret);

	mutex_unlock(&g_hsm_work_mutex);
	return ret;
}
EXPORT_SYMBOL(clear_flash_resetcnt);

int sec_get_fw_verify_result(u32 dev_id, u32 partition_type, u32 *result)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	if (!result || partition_type > 1) {
		pr_err("3[HSM firmware]:[%d] invalid params.\n", __LINE__);
		return HSM_FW_INVALID_RET;
	}

	ret = firmware_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] init and open sess failed, 0x%x.\n",
			__LINE__, ret);
		return (int)ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_PARAM_21;
	op.params[0].value.a = dev_id;
	op.params[0].value.b = partition_type;

	ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_GET_FW_VERIFY_RESULT, &op, &origin);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] get img soc verify info, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);
	} else {
		*result = op.params[1].value.a;
	}

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(sec_get_fw_verify_result);

int sec_img_update(u32 dev_id, u32 img_index)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	if (img_index > HSM_FW_MAX_IMG_NUM) {
		pr_err("3[HSM firmware]:[%d] invalid params, 0x%x.\n", __LINE__, img_index);
		return HSM_FW_INVALID_RET;
	}

	ret = firmware_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] init and open sess fail, 0x%x.\n",
			__LINE__, ret);
		return (int)ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_PARAM_11;
	op.params[0].value.a = dev_id;
	op.params[1].value.a = img_index;

	ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_IMG_UPDATE, &op, &origin);
	if (ret)
		pr_err("3[HSM firmware]:[%d] sec update img %d failed, 0x%x, 0x%x.\n",
			__LINE__, img_index, ret, origin);

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(sec_img_update);

int sec_update_finish(u32 dev_id)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	ret = firmware_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] init and open sess fail, 0x%x.\n",
			__LINE__, ret);
		return (int)ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_PARAM_11;
	op.params[0].value.a = dev_id;

	ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_UPDATE_FINISH, &op, &origin);
	if (ret)
		pr_err("3[HSM firmware]:[%d] sec update finish failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(sec_update_finish);

int sec_img_sync_and_efuse_update(u32 dev_id)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	ret = firmware_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] init and open sess fail, 0x%x.\n",
			__LINE__, ret);
		return (int)ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_PARAM_11;
	op.params[0].value.a = dev_id;

	ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_SYNC_AND_EFUSE_UPDATE, &op, &origin);
	if (ret)
		pr_err("3[HSM firmware]:[%d] sec img sync & efuse update failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(sec_img_sync_and_efuse_update);

int sec_rim_update(u32 dev_id, u8 *rim, u32 rim_len)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	if (!rim || rim_len != HSM_FW_RIM_LEN) {
		pr_err("3[HSM firmware]:[%d] invalid params, 0x%x.\n", __LINE__, rim_len);
		return HSM_FW_INVALID_RET;
	}

	ret = firmware_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] init and open sess fail, 0x%x.\n",
			__LINE__, ret);
		return (int)ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_PARAM_51;
	op.params[0].value.a = dev_id;
	op.params[2].tmpref.buffer = rim;
	op.params[2].tmpref.size = rim_len;

	ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_RIM_UPDATE, &op, &origin);
	if (ret)
		pr_err("3[HSM firmware]:[%d] rim update failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(sec_rim_update);

int sec_flash_read_cmdline(u32 dev_id, u32 *out_value)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	if (!out_value) {
		pr_err("3[HSM firmware]:[%d] invalid params.\n", __LINE__);
		return HSM_FW_INVALID_RET;
	}

	ret = firmware_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] init and open sess fail, 0x%x.\n",
			__LINE__, ret);
		return (int)ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_PARAM_21;
	op.params[0].value.a = dev_id;

	ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_FLASH_READ_CMDLINE, &op, &origin);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] get flash cmd value failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);
	} else {
		*out_value = op.params[1].value.a;
	}

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(sec_flash_read_cmdline);

int sec_reset_recovery_boot_count(u32 dev_id)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	ret = firmware_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] init and open sess fail, 0x%x.\n",
			__LINE__, ret);
		return (int)ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_PARAM_11;
	op.params[0].value.a = dev_id;

	ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_RESET_RECOVERY_BOOT_COUNT, &op, &origin);
	if (ret)
		pr_err("3[HSM firmware]:[%d] reset recovery boot cnt failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(sec_reset_recovery_boot_count);

int sec_flash_get_count(u32 dev_id, u32 *pflash_cnt)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	if (!pflash_cnt) {
		pr_err("3[HSM firmware]:[%d] invalid params.\n", __LINE__);
		return HSM_FW_INVALID_RET;
	}

	ret = firmware_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] init and open sess fail, 0x%x.\n",
			__LINE__, ret);
		return (int)ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_PARAM_21;
	op.params[0].value.a = dev_id;

	ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_FLASH_GET_COUNT, &op, &origin);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] get sec flash count failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);
	} else {
		*pflash_cnt = op.params[1].value.a;
	}

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(sec_flash_get_count);

int sec_flash_get_info(u32 dev_id, u32 flash_idx, sec_flash_info_t *flash_info)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	if (!flash_info || flash_idx > 1) {
		pr_err("3[HSM firmware]:[%d] Invalid params, 0x%x.\n", __LINE__, flash_idx);
		return HSM_FW_INVALID_RET;
	}

	ret = firmware_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] init and open sess fail, 0x%x.\n",
			__LINE__, ret);
		return (int)ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_PARAM_611;
	op.params[0].value.a = dev_id;
	op.params[1].value.a = flash_idx;
	op.params[2].tmpref.buffer = flash_info;
	op.params[2].tmpref.size = sizeof(*flash_info);

	ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_FLASH_GET_INFO, &op, &origin);
	if (ret)
		pr_err("3[HSM firmware]:[%d] sec flash get info failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(sec_flash_get_info);

int sec_ufsck_key_cfg(u32 dev_id, u32 key_num, sec_ufsck_s *pbkdf2_info, u32 derive_info_size)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	if (!pbkdf2_info || derive_info_size != sizeof(*pbkdf2_info)) {
		pr_err("3[HSM firmware]:[%d] invalid params, 0x%x.\n", __LINE__, derive_info_size);
		return HSM_FW_INVALID_RET;
	}

	ret = firmware_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] init and open sess fail, 0x%x.\n",
			__LINE__, ret);
		return (int)ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_PARAM_51;
	op.params[0].value.a = key_num;
	op.params[2].tmpref.buffer = pbkdf2_info;
	op.params[2].tmpref.size = derive_info_size;

	ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_UFSCK_KEY_CFG, &op, &origin);
	if (ret)
		pr_err("3[HSM firmware]:[%d] ufs ck key derive failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(sec_ufsck_key_cfg);

int32_t sec_read_nsforbid(u32 dev_id, u32 *nsforbid_val)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	if (!nsforbid_val) {
		pr_err("3[HSM firmware]:[%d] invalid params.\n", __LINE__);
		return HSM_FW_INVALID_RET;
	}

	*nsforbid_val = 0xff;
	ret = firmware_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] init and open sess fail, 0x%x.\n",
			__LINE__, ret);
		return 0;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_PARAM_21;
	op.params[0].value.a = dev_id;

	ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_READ_NSFORBID, &op, &origin);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] read sec uefi nsforbid failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);
	} else if (op.params[1].value.a == 0) {
		*nsforbid_val = 0;
	}

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return 0;
}
EXPORT_SYMBOL(sec_read_nsforbid);

int soc_get_nvcnt(u32 dev_id, u32 *buf, u32 size)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 value = 0;
	u32 ret;
	int copy_ret;

	if (!buf || size < HSM_FW_NV_CNT_SIZE) {
		pr_err("3[HSM firmware]:[%d] invalid params, 0x%x.\n", __LINE__, size);
		return HSM_FW_INVALID_RET;
	}

	ret = firmware_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] init and open sess fail, 0x%x.\n",
			__LINE__, ret);
		return (int)ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_PARAM_16;
	op.params[0].value.a = dev_id;
	op.params[1].tmpref.buffer = &value;
	op.params[1].tmpref.size = sizeof(value);

	ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_SOC_GET_NVCNT, &op, &origin);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] soc get nvcnt failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);
		hsm_teek_close_session(&session);
		hsm_teek_finalize_ctx(&ctx);
		return (int)ret;
	}

	copy_ret = memcpy_s(buf, size, &value, sizeof(value));
	if (copy_ret) {
		pr_err("3[HSM firmware]:[%d] mem cpy failed, 0x%x.\n", __LINE__, size);
		hsm_teek_close_session(&session);
		hsm_teek_finalize_ctx(&ctx);
		return HSM_FW_COPY_ERR;
	}

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return 0;
}
EXPORT_SYMBOL(soc_get_nvcnt);

int sec_flash_get_version(u32 dev_id, u32 img_id, u8 *boot_version, u32 max_len, u32 area_check)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u8 *tmp_buf;
	u32 origin = 0;
	u32 ret;
	int copy_ret;

	if (!boot_version || max_len <= (HSM_FW_VERSION_LEN - 1) || area_check > 1) {
		pr_err("3[HSM firmware]:[%d] invalid params, 0x%x, 0x%x.\n",
			__LINE__, max_len, area_check);
		return HSM_FW_INVALID_RET;
	}

	ret = firmware_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] init and open sess fail, 0x%x.\n",
			__LINE__, ret);
		return (int)ret;
	}

	tmp_buf = kmalloc(HSM_FW_VERSION_LEN, GFP_KERNEL);
	if (!tmp_buf) {
		pr_err("3[HSM firmware]:[%d] malloc buf failed.\n", __LINE__);
		hsm_teek_close_session(&session);
		hsm_teek_finalize_ctx(&ctx);
		return HSM_FW_NOMEM_ERR;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_PARAM_611;
	op.params[0].value.a = dev_id;
	op.params[1].value.a = img_id;
	op.params[1].value.b = area_check;
	op.params[2].tmpref.buffer = tmp_buf;
	op.params[2].tmpref.size = HSM_FW_VERSION_LEN;

	ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_FLASH_GET_VERSION, &op, &origin);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] get sec flash version failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);
		kfree(tmp_buf);
		hsm_teek_close_session(&session);
		hsm_teek_finalize_ctx(&ctx);
		return (int)ret;
	}

	copy_ret = memcpy_s(boot_version, max_len, tmp_buf, HSM_FW_VERSION_LEN);
	kfree(tmp_buf);
	if (copy_ret) {
		pr_err("3[HSM firmware]:[%d] mem cpy failed, 0x%x.\n", __LINE__, max_len);
		hsm_teek_close_session(&session);
		hsm_teek_finalize_ctx(&ctx);
		return HSM_FW_COPY_ERR;
	}

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return 0;
}
EXPORT_SYMBOL(sec_flash_get_version);

int sec_img_verify(u32 dev_id, u32 img_num, img_verify_info_s *img_info, u32 pss_cfg)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	int ret;
	u32 i;

	if (!img_info || img_num == 0 || img_num > HSM_FW_MAX_IMG_NUM || device_id_check(dev_id)) {
		pr_err("3[HSM firmware]:[%d] invalid params, 0x%x,0x%x.\n",
			__LINE__, img_num, dev_id);
		return HSM_FW_INVALID_RET;
	}

	mutex_lock(g_upgrade_img[dev_id]);

	ret = firmware_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM firmware]:[%d] init and open sess failed, 0x%x.\n",
			__LINE__, ret);
		goto out_unlock;
	}

	ret = sec_img_sync_before_update(dev_id, &session);
	if (ret)
		pr_warn("4[HSM firmware]:[%d] sync before update failed, 0x%x\n",
			__LINE__, ret);

	if (!g_img_buf[dev_id]) {
		ret = hsm_fw_alloc_img_buf(dev_id);
		if (ret)
			goto out_close;
	}

	for (i = 0; i < img_num; i++) {
		dma_addr_t phys;
		int copy_ret;

		memset(&op, 0, sizeof(op));
		if (img_info[i].img_len > HSM_FW_MAX_IMG_SIZE) {
			pr_err("3[HSM firmware]:[%d] invalid img(%d) size 0x%x.\n",
				__LINE__, i, img_info[i].img_len);
			ret = HSM_FW_INVALID_RET;
			goto out_close;
		}

		copy_ret = memcpy_s(g_img_buf[dev_id], HSM_FW_MAX_IMG_SIZE,
			img_info[i].image_buf, img_info[i].img_len);
		if (copy_ret) {
			pr_err("3[HSM firmware]:[%d] copy image failed, 0x%x.\n",
				__LINE__, copy_ret);
			ret = HSM_FW_INVALID_RET;
			goto out_close;
		}

		phys = virt_to_phys(g_img_buf[dev_id]);
		op.started = 1;
		op.param_types = HSM_PARAM_111;
		op.params[0].value.a = dev_id;
		op.params[0].value.b = pss_cfg;
		op.params[1].value.a = img_info[i].img_id;
		op.params[1].value.b = img_info[i].img_len;
		op.params[2].value.a = lower_32_bits(phys);
		op.params[2].value.b = upper_32_bits(phys);

		ret = hsm_teek_invoke_command(&session, HSM_FW_CMD_IMG_VERIFY, &op, NULL);
		if (ret) {
			pr_err("3[HSM firmware]:[%d] sec img (%d) verify failed, 0x%x, 0x%x.\n",
				__LINE__, i, ret, 0);
			goto out_close;
		}
	}

	ret = 0;

out_close:
	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
out_unlock:
	mutex_unlock(g_upgrade_img[dev_id]);
	return ret;
}
EXPORT_SYMBOL(sec_img_verify);

static int __init hsm_firmware_update_init(void)
{
	pr_info("6[HSM firmware]:[%d] Driver initialization.\n", __LINE__);

	g_workqueue = alloc_workqueue("hsm-ca-update-firmware",
		WQ_UNBOUND | WQ_MEM_RECLAIM, 1);
	if (!g_workqueue) {
		pr_err("3[HSM firmware]:[%d] work queue creat failed.\n", __LINE__);
		return -ENOMEM;
	}

	mutex_init(&g_hsm_work_mutex);
	mutex_init(g_upgrade_img[0]);
	mutex_init(g_upgrade_img[1]);
	INIT_WORK(&g_hsm_work.work, clear_flash_resetcnt_wq);
	init_completion(&g_hsm_work.done);
	g_hsm_work.result = 0;
	g_hsm_work.dev_id = 0;

	clear_flash_resetcnt(0);
	return 0;
}

static void __exit hsm_firmware_update_exit(void)
{
	pr_info("6[HSM firmware]:[%d] Driver de-initialization\n", __LINE__);

	cancel_work_sync(&g_hsm_work.work);
	if (g_workqueue) {
		flush_workqueue(g_workqueue);
		destroy_workqueue(g_workqueue);
		g_workqueue = NULL;
	}

	kfree(g_img_buf[0]);
	g_img_buf[0] = NULL;
	kfree(g_img_buf[1]);
	g_img_buf[1] = NULL;
}

module_init(hsm_firmware_update_init);
module_exit(hsm_firmware_update_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Reconstructed HSM firmware update module");
MODULE_AUTHOR("OpenAI");
