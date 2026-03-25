// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/cred.h>
#include <linux/uidgid.h>
#include <linux/dcache.h>
#include <linux/file.h>
#include <linux/path.h>
#include <linux/sched.h>

#include "hsm_norflash.h"

#define HSM_FLASH_CHUNK_SIZE       0x100000U
#define HSM_FLASH_INVALID_RET      1
#define HSM_TEEK_SYMBOL_ERR        0xFFFF0007U

#define HSM_FLASH_CMD_WRITE        0x9000U
#define HSM_FLASH_CMD_RAW_WRITE    0x9001U
#define HSM_FLASH_CMD_READ         0x9002U
#define HSM_FLASH_CMD_ERASE        0x9003U
#define HSM_FLASH_CMD_IO_WRITE32   0x900AU
#define HSM_FLASH_CMD_IO_READ32    0x900BU

#define HSM_FLASH_PARAM_11         0x11U
#define HSM_FLASH_PARAM_21         0x21U
#define HSM_FLASH_PARAM_511        0x511U
#define HSM_FLASH_PARAM_611        0x611U
#define HSM_OPEN_SESSION_PARAM_55  0x55U
#define HSM_TEEK_LOGIN_IDENTIFY    7U

#define HSM_FLASH_PATH_MAX         260U

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

struct hsm_flash_open_details {
	u32 uid;
	char path[HSM_FLASH_PATH_MAX];
};

struct hsm_flash_partition {
	u32 part_id;
	u32 flash_addr;
	u32 mtd_size;
	char mtd_name[MAX_MTD_NAME_SIZE];
};

struct hsm_flash_dev {
	struct device *dev;
	struct miscdevice miscdev;
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

static const u8 g_flash_uuid[16] = {
	0x30, 0x9c, 0xf9, 0xe3, 0x9d, 0xf1, 0xf8, 0x41,
	0x87, 0xc2, 0xe0, 0x27, 0x4f, 0x59, 0xcd, 0x26,
};

static const struct hsm_flash_partition g_flash_partitions[] = {
	{ .part_id = 0,  .flash_addr = 0x00000000, .mtd_size = 0x00040000, .mtd_name = "flashboot" },
	{ .part_id = 21, .flash_addr = 0x00040000, .mtd_size = 0x00010000, .mtd_name = "Img_upgrade_flag_0" },
	{ .part_id = 22, .flash_addr = 0x00050000, .mtd_size = 0x00010000, .mtd_name = "Img_upgrade_flag_1" },
	{ .part_id = 1,  .flash_addr = 0x00060000, .mtd_size = 0x00020000, .mtd_name = "Hboot1_a_0" },
	{ .part_id = 3,  .flash_addr = 0x00080000, .mtd_size = 0x00020000, .mtd_name = "Hboot1_a_1" },
	{ .part_id = 2,  .flash_addr = 0x000a0000, .mtd_size = 0x00020000, .mtd_name = "Hlink_0" },
	{ .part_id = 4,  .flash_addr = 0x000c0000, .mtd_size = 0x00020000, .mtd_name = "Hlink_1" },
	{ .part_id = 5,  .flash_addr = 0x000e0000, .mtd_size = 0x00040000, .mtd_name = "Hboot1_b_0" },
	{ .part_id = 6,  .flash_addr = 0x00120000, .mtd_size = 0x00040000, .mtd_name = "Hboot1_b_1" },
	{ .part_id = 17, .flash_addr = 0x00160000, .mtd_size = 0x00060000, .mtd_name = "SYSBASE_CFG_0" },
	{ .part_id = 18, .flash_addr = 0x001c0000, .mtd_size = 0x00060000, .mtd_name = "SYSBASE_CFG_1" },
	{ .part_id = 11, .flash_addr = 0x00220000, .mtd_size = 0x00080000, .mtd_name = "HSM_0" },
	{ .part_id = 12, .flash_addr = 0x002a0000, .mtd_size = 0x00080000, .mtd_name = "HSM_1" },
	{ .part_id = 9,  .flash_addr = 0x00320000, .mtd_size = 0x00050000, .mtd_name = "DDR_0" },
	{ .part_id = 10, .flash_addr = 0x00370000, .mtd_size = 0x00050000, .mtd_name = "DDR_1" },
	{ .part_id = 40, .flash_addr = 0x003c0000, .mtd_size = 0x00200000, .mtd_name = "ATF_0" },
	{ .part_id = 41, .flash_addr = 0x005c0000, .mtd_size = 0x00200000, .mtd_name = "ATF_1" },
	{ .part_id = 7,  .flash_addr = 0x00800000, .mtd_size = 0x00400000, .mtd_name = "Hboot2_0" },
	{ .part_id = 8,  .flash_addr = 0x00c00000, .mtd_size = 0x00400000, .mtd_name = "Hboot2_1" },
	{ .part_id = 42, .flash_addr = 0x01b00000, .mtd_size = 0x00100000, .mtd_name = "USERBASE_CFG_0" },
	{ .part_id = 43, .flash_addr = 0x01c00000, .mtd_size = 0x00100000, .mtd_name = "USERBASE_CFG_1" },
};

static const struct of_device_id g_hsm_flash_match[] = {
	{ .compatible = "sec,hsm_flash" },
	{ }
};
MODULE_DEVICE_TABLE(of, g_hsm_flash_match);

static void *hsm_flash_symbol_get(const char *name)
{
	void *sym = __symbol_get(name);

	if (!sym)
		pr_err("3[HSM_AUTH]: get symbol %s failed.\n", name);

	return sym;
}

static u32 hsm_teek_initialize_ctx(struct hsm_teek_context *ctx)
{
	if (!g_teek_init)
		g_teek_init = (hsm_teek_init_fn)hsm_flash_symbol_get("TEEK_InitializeContext");
	if (!g_teek_init)
		return HSM_TEEK_SYMBOL_ERR;
	return g_teek_init(NULL, ctx);
}

static void hsm_teek_finalize_ctx(struct hsm_teek_context *ctx)
{
	if (!g_teek_fini)
		g_teek_fini = (hsm_teek_fini_fn)hsm_flash_symbol_get("TEEK_FinalizeContext");
	if (g_teek_fini)
		g_teek_fini(ctx);
}

static u32 hsm_teek_open_session(struct hsm_teek_context *ctx,
	struct hsm_teek_session *session, struct hsm_teek_operation *operation,
	u32 *origin)
{
	if (!g_teek_open)
		g_teek_open = (hsm_teek_open_fn)hsm_flash_symbol_get("TEEK_OpenSession");
	if (!g_teek_open)
		return HSM_TEEK_SYMBOL_ERR;

	return g_teek_open(ctx, session, g_flash_uuid, HSM_TEEK_LOGIN_IDENTIFY,
		NULL, operation, origin);
}

static void hsm_teek_close_session(struct hsm_teek_session *session)
{
	if (!g_teek_close)
		g_teek_close = (hsm_teek_close_fn)hsm_flash_symbol_get("TEEK_CloseSession");
	if (g_teek_close)
		g_teek_close(session);
}

static u32 hsm_teek_invoke_command(struct hsm_teek_session *session, u32 command,
	struct hsm_teek_operation *operation, u32 *origin)
{
	if (!g_teek_invoke)
		g_teek_invoke = (hsm_teek_invoke_fn)hsm_flash_symbol_get("TEEK_InvokeCommand");
	if (!g_teek_invoke)
		return HSM_TEEK_SYMBOL_ERR;

	return g_teek_invoke(session, command, operation, origin);
}

static int hsm_get_process_details(struct hsm_flash_open_details *details)
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

static int hsm_init_and_open_session(u32 dev_id, struct hsm_teek_context *ctx,
	struct hsm_teek_session *session)
{
	struct hsm_flash_open_details details;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;
	size_t path_len;
	int detail_ret;

	if (dev_id > 1) {
		pr_err("3[HSM FLASH]:[%d] dev id 0x%x is invalid.\n", __LINE__, dev_id);
		return HSM_FLASH_INVALID_RET;
	}

	ret = hsm_teek_initialize_ctx(ctx);
	if (ret) {
		pr_err("3[HSM FLASH]:[%d] TEEK initialize context failed, 0x%x.\n", __LINE__, ret);
		return (int)ret;
	}

	detail_ret = hsm_get_process_details(&details);
	if (detail_ret) {
		pr_err("3[HSM FLASH]:[%d] TEEK get process detail failed, 0x%x.\n",
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

	ret = hsm_teek_open_session(ctx, session, &op, &origin);
	if (ret) {
		pr_err("3[HSM FLASH]:[%d] TEEK open session failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);
		hsm_teek_finalize_ctx(ctx);
	}

	return (int)ret;
}

static int sec_flash_operation(u32 dev_id, u32 flash_offset, void *buf, u32 buf_len,
	u32 command, u32 param_types)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;
	u32 remain = buf_len;
	u32 offset = flash_offset;
	u8 *cursor = buf;

	if (!buf || buf_len == 0 || buf_len > 0x1000000U) {
		pr_err("3[HSM FLASH]:[%d] flash write parms invalid, 0x%x.\n",
			__LINE__, buf_len);
		return HSM_FLASH_INVALID_RET;
	}

	ret = hsm_init_and_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM FLASH]:[%d] init and open sess fail, 0x%x\n", __LINE__, ret);
		return (int)ret;
	}

	while (remain > 0) {
		u32 chunk = min(remain, HSM_FLASH_CHUNK_SIZE);

		memset(&op, 0, sizeof(op));
		op.started = 1;
		op.param_types = param_types;
		op.params[0].value.a = dev_id;
		op.params[1].value.a = offset;
		op.params[2].tmpref.buffer = cursor;
		op.params[2].tmpref.size = chunk;

		ret = hsm_teek_invoke_command(&session, command, &op, &origin);
		if (ret) {
			pr_err("3[HSM FLASH]:[%d] flash operation failed, 0x%x, 0x%x.\n",
				__LINE__, ret, origin);
			break;
		}

		remain -= chunk;
		offset += chunk;
		cursor += chunk;
	}

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}

int sec_flash_write(u32 dev_id, u32 flash_offset, const u8 *buf, u32 buf_len)
{
	u32 ret = sec_flash_operation(dev_id, flash_offset, (void *)buf, buf_len,
		HSM_FLASH_CMD_WRITE, HSM_FLASH_PARAM_511);

	if (ret)
		pr_err("3[HSM FLASH]:[%d] flash write failed, 0x%x.\n", __LINE__, ret);
	return (int)ret;
}
EXPORT_SYMBOL(sec_flash_write);

int sec_flash_raw_write(u32 dev_id, u32 flash_offset, const u8 *buf, u32 buf_len)
{
	u32 ret = sec_flash_operation(dev_id, flash_offset, (void *)buf, buf_len,
		HSM_FLASH_CMD_RAW_WRITE, HSM_FLASH_PARAM_511);

	if (ret)
		pr_err("3[HSM FLASH]:[%d] flash raw write failed, 0x%x.\n", __LINE__, ret);
	return (int)ret;
}
EXPORT_SYMBOL(sec_flash_raw_write);

int sec_flash_read(u32 dev_id, u32 flash_offset, u8 *buf, u32 buf_len)
{
	u32 ret = sec_flash_operation(dev_id, flash_offset, buf, buf_len,
		HSM_FLASH_CMD_READ, HSM_FLASH_PARAM_611);

	if (ret)
		pr_err("3[HSM FLASH]:[%d] flash read failed, 0x%x.\n", __LINE__, ret);
	return (int)ret;
}
EXPORT_SYMBOL(sec_flash_read);

int sec_flash_erase(u32 dev_id, u32 flash_offset, u32 buf_len)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	if (buf_len == 0 || buf_len > 0x1000000U) {
		pr_err("3[HSM FLASH]:[%d] erase length invalid, 0x%x.\n", __LINE__, buf_len);
		return HSM_FLASH_INVALID_RET;
	}

	ret = hsm_init_and_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM FLASH]:[%d] init and open sess fail, 0x%x.\n", __LINE__, ret);
		return (int)ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_FLASH_PARAM_11;
	op.params[0].value.a = dev_id;
	op.params[1].value.a = flash_offset;
	op.params[1].value.b = buf_len;

	ret = hsm_teek_invoke_command(&session, HSM_FLASH_CMD_ERASE, &op, &origin);
	if (ret)
		pr_err("3[HSM FLASH]:[%d] flash operation failed, 0x%x, 0x%x.\n",
			__LINE__, ret, origin);

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(sec_flash_erase);

int32_t sec_io_write32(u32 dev_id, SEC_IO_REGS_ID_TYPE type, u32 val)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	ret = hsm_init_and_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM IO]:[%d] init and open sess fail, 0x%x\n", __LINE__, ret);
		return (int)ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_FLASH_PARAM_11;
	op.params[0].value.a = dev_id;
	op.params[1].value.a = (u32)type;
	op.params[1].value.b = val;

	ret = hsm_teek_invoke_command(&session, HSM_FLASH_CMD_IO_WRITE32, &op, &origin);
	if (ret)
		pr_err("3[HSM IO]:[%d] write32 failed, 0x%x, 0x%x.\n", __LINE__, ret, origin);

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(sec_io_write32);

int32_t sec_io_read32(u32 dev_id, SEC_IO_REGS_ID_TYPE type, u32 *val)
{
	struct hsm_teek_context ctx;
	struct hsm_teek_session session;
	struct hsm_teek_operation op;
	u32 origin = 0;
	u32 ret;

	if (!val) {
		pr_err("3[HSM IO]:[%d] val is NULL.\n", __LINE__);
		return HSM_FLASH_INVALID_RET;
	}

	ret = hsm_init_and_open_session(dev_id, &ctx, &session);
	if (ret) {
		pr_err("3[HSM IO]:[%d] init and open sess fail, 0x%x\n", __LINE__, ret);
		return (int)ret;
	}

	memset(&op, 0, sizeof(op));
	op.started = 1;
	op.param_types = HSM_FLASH_PARAM_21;
	op.params[0].value.a = dev_id;
	op.params[0].value.b = (u32)type;

	ret = hsm_teek_invoke_command(&session, HSM_FLASH_CMD_IO_READ32, &op, &origin);
	if (!ret)
		*val = op.params[1].value.a;
	else
		pr_err("3[HSM IO]:[%d] read32 failed, 0x%x, 0x%x.\n", __LINE__, ret, origin);

	hsm_teek_close_session(&session);
	hsm_teek_finalize_ctx(&ctx);
	return (int)ret;
}
EXPORT_SYMBOL(sec_io_read32);

int sec_flash_read_mtd_nums(u32 *nums)
{
	if (!nums) {
		pr_err("3[HSM FLASH]:[%d] mtd nums is NULL.\n", __LINE__);
		return HSM_FLASH_INVALID_RET;
	}

	*nums = ARRAY_SIZE(g_flash_partitions);
	return 0;
}
EXPORT_SYMBOL(sec_flash_read_mtd_nums);

int sec_flash_read_mtd_info(u32 read_count, sec_flash_mtd_info_s *mtd_info)
{
	u32 i;

	if (!mtd_info || read_count == 0 || read_count > ARRAY_SIZE(g_flash_partitions)) {
		pr_err("3[HSM FLASH]:[%d] mtd info invalid, 0x%x.\n", __LINE__, read_count);
		return HSM_FLASH_INVALID_RET;
	}

	for (i = 0; i < read_count; i++) {
		mtd_info[i].mtd_size = g_flash_partitions[i].mtd_size;
		mtd_info[i].flash_addr = g_flash_partitions[i].flash_addr;
		if (strscpy(mtd_info[i].mtd_name, g_flash_partitions[i].mtd_name,
				sizeof(mtd_info[i].mtd_name)) < 0) {
			pr_err("3[HSM FLASH]:[%d] str cpy failed.\n", __LINE__);
			return HSM_FLASH_INVALID_RET;
		}
	}

	return 0;
}
EXPORT_SYMBOL(sec_flash_read_mtd_info);

static long hsm_flash_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	sec_flash_s flash_req;
	sec_flash_info_s info_req;
	u8 *kbuf = NULL;
	int ret = 0;

	switch (cmd) {
	case FLASH_IOCTL_CMD_READ:
		if (copy_from_user(&flash_req, argp, sizeof(flash_req)))
			return -EFAULT;
		kbuf = kmalloc(flash_req.buf_len, GFP_KERNEL);
		if (!kbuf)
			return -ENOMEM;
		ret = sec_flash_read(flash_req.dev_id, flash_req.flash_addr, kbuf, flash_req.buf_len);
		if (!ret && copy_to_user((void __user *)(uintptr_t)flash_req.buf, kbuf, flash_req.buf_len))
			ret = -EFAULT;
		kfree(kbuf);
		return ret;
	case FLASH_IOCTL_CMD_WRITE:
	case FLASH_IOCTL_CMD_RAW_WRITE:
		if (copy_from_user(&flash_req, argp, sizeof(flash_req)))
			return -EFAULT;
		kbuf = memdup_user((const void __user *)(uintptr_t)flash_req.buf, flash_req.buf_len);
		if (IS_ERR(kbuf))
			return PTR_ERR(kbuf);
		if (cmd == FLASH_IOCTL_CMD_WRITE)
			ret = sec_flash_write(flash_req.dev_id, flash_req.flash_addr, kbuf, flash_req.buf_len);
		else
			ret = sec_flash_raw_write(flash_req.dev_id, flash_req.flash_addr, kbuf, flash_req.buf_len);
		kfree(kbuf);
		return ret;
	case FLASH_IOCTL_CMD_ERASE:
		if (copy_from_user(&flash_req, argp, sizeof(flash_req)))
			return -EFAULT;
		return sec_flash_erase(flash_req.dev_id, flash_req.flash_addr, flash_req.buf_len);
	case FLASH_IOCTL_CMD_GET_MTD_INFO:
		if (copy_from_user(&info_req, argp, sizeof(info_req)))
			return -EFAULT;
		ret = sec_flash_read_mtd_info(info_req.read_count, &info_req.flash_info);
		if (!ret && copy_to_user(argp, &info_req, sizeof(info_req)))
			ret = -EFAULT;
		return ret;
	default:
		pr_err("3[HSM FLASH]:[%d] ioctl cmd invalid, 0x%x.\n", __LINE__, cmd);
		return -EINVAL;
	}
}

static const struct file_operations g_flash_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = hsm_flash_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = hsm_flash_ioctl,
#endif
};

static int hsm_flash_probe(struct platform_device *pdev)
{
	struct hsm_flash_dev *flash_dev;
	int ret;

	flash_dev = devm_kzalloc(&pdev->dev, sizeof(*flash_dev), GFP_KERNEL);
	if (!flash_dev)
		return -ENOMEM;

	flash_dev->dev = &pdev->dev;
	flash_dev->miscdev.minor = MISC_DYNAMIC_MINOR;
	flash_dev->miscdev.name = "hsm_flash";
	flash_dev->miscdev.fops = &g_flash_fops;

	platform_set_drvdata(pdev, flash_dev);

	ret = misc_register(&flash_dev->miscdev);
	if (ret) {
		pr_err("3[HSM FLASH]:[%d] misc register failed, 0x%x.\n", __LINE__, ret);
		return ret;
	}

	pr_info("6[HSM FLASH]:[%d] Driver initialization OK.\n", __LINE__);
	return 0;
}

static void hsm_flash_remove(struct platform_device *pdev)
{
	struct hsm_flash_dev *flash_dev = platform_get_drvdata(pdev);

	if (flash_dev)
		misc_deregister(&flash_dev->miscdev);
}

static void hsm_flash_device_release(struct device *dev)
{
}

static struct platform_driver g_hsm_flash_driver = {
	.probe = hsm_flash_probe,
	.remove = hsm_flash_remove,
	.driver = {
		.name = "hsm_flash",
		.of_match_table = g_hsm_flash_match,
	},
};

static struct platform_device g_hsm_flash_pdev = {
	.name = "hsm_flash",
	.id = -1,
	.dev = {
		.release = hsm_flash_device_release,
	},
};

static int __init hsm_flash_init(void)
{
	int ret;

	pr_info("6[HSM FLASH]:[%d] Driver initialization.\n", __LINE__);

	ret = platform_driver_register(&g_hsm_flash_driver);
	if (ret) {
		pr_err("3[HSM FLASH]:[%d] platform driver register failed, 0x%x.\n",
			__LINE__, ret);
		return ret;
	}

	ret = platform_device_register(&g_hsm_flash_pdev);
	if (ret) {
		pr_err("3[HSM FLASH]:[%d] platform device register failed, 0x%x.\n",
			__LINE__, ret);
		platform_driver_unregister(&g_hsm_flash_driver);
	}

	return ret;
}

static void __exit hsm_flash_exit(void)
{
	pr_info("6[HSM FLASH]:[%d] Driver de-initialization\n", __LINE__);
	platform_device_unregister(&g_hsm_flash_pdev);
	platform_driver_unregister(&g_hsm_flash_driver);
}

module_init(hsm_flash_init);
module_exit(hsm_flash_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Reconstructed HSM flash transport module");
MODULE_AUTHOR("OpenAI");
