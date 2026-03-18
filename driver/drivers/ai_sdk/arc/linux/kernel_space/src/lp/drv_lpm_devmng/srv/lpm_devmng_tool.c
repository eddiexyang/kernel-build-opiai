/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * @file    lpm_devmng_tool.c
 * @brief   entry for lptest_tool module
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/stddef.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/notifier.h>
#include <linux/securec.h>

#include "icm_interface.h"
#include "lpm_devmng_ipc.h"
#include "lpm_devmng_turbo.h"
#include "lpm_devmng_tool.h"

#ifdef LPM_BUILD_DEBUG

#define LP_NAME           "lptest"
#define LPM_LPTEST_TIMEOUT HZ   /* 1s */

STATIC uint32_t g_log_length[LPM_DEVMNG_DEV_MAX_NUM];
STATIC uint8_t g_log_buffer[LPM_DEVMNG_DEV_MAX_NUM][LPTEST_RESULT_BUFF_LEN] = {0};
STATIC struct semaphore g_lptest_set_sem[LPM_DEVMNG_DEV_MAX_NUM];
STATIC struct semaphore g_lptest_get_sem[LPM_DEVMNG_DEV_MAX_NUM];
STATIC struct lpm_lptest_dev_cfg g_lptest_dev = {0};
STATIC struct lpm_tool_cmd g_tool_cmd[LPM_DEVMNG_DEV_MAX_NUM];

STATIC int32_t lpm_sr_test_init(void);
STATIC void lpm_sr_test_exit(void);
STATIC int32_t execute_lptest_cmd(uint32_t dev_id, const struct lpm_tool_cmd *tool_cmd);
STATIC int32_t execute_suspend_cmd(uint32_t dev_id, const struct lpm_tool_cmd *tool_cmd);
STATIC int32_t execute_quick_cmd(uint32_t dev_id, const struct lpm_tool_cmd *tool_cmd);

STATIC bool tool_cmd_format(struct lpm_tool_cmd *tool_cmd, uint32_t total_len)
{
	struct lpm_lptest_quick_cmd *quick_cmd = &tool_cmd->cmd_data.quick_cmd;
	struct lpm_lptest_cmd *lptest_cmd = &tool_cmd->cmd_data.lptest_cmd;
	if (total_len >= sizeof(struct lpm_lptest_quick_cmd)) {
		if (quick_cmd->magic == LPM_QUICK_CMD_MAGIC) {
			if (quick_cmd->data_len == (total_len - sizeof(struct lpm_lptest_quick_cmd))) {
				tool_cmd->cmd_type = LPM_CMD_TYPE_QUICK;
				return true;
			} else {
				lpm_log_err("quick cmd invalid, total_len=%u, data_len=%u\n",
					total_len, quick_cmd->data_len);
				return false;
			}
		}
	}
	if (total_len == sizeof(struct lpm_lptest_cmd)) {
		lpm_log_info("lptest cmd(%s)\n", lptest_cmd->cmd_name);
		if (strcmp(lptest_cmd->cmd_name, "suspend") == 0) {
			tool_cmd->cmd_type = LPM_CMD_TYPE_SUSPEND;
			return true;
		} else {
			tool_cmd->cmd_type = LPM_CMD_TYPE_LPTEST;
			return true;
		}
	}

	lpm_log_err("tool cmd invalid, tool_len=%u\n", total_len);
	return false;
}

// process tool cmd in buffer, save result in global buffer
STATIC int32_t execute_tool_cmd(uint32_t dev_id, const char *in, uint32_t in_len)
{
	int32_t ret;
	static const lpm_tool_cmd_handler cmd_handler[LPM_CMD_TYPE_MAX] = {
		[LPM_CMD_TYPE_LPTEST]  = execute_lptest_cmd,
		[LPM_CMD_TYPE_SUSPEND] = execute_suspend_cmd,
		[LPM_CMD_TYPE_QUICK] = execute_quick_cmd,
	};
	struct lpm_tool_cmd *tool_cmd = &g_tool_cmd[dev_id];

	if ((in == NULL) || (in_len == 0) || (in_len > LPM_TOOL_CMD_RAW_SIZE)) {
		lpm_log_err("lptest cmd buff invalid, in_len=%u\n", in_len);
		return -1;
	}

	(void)memset_s(tool_cmd, sizeof(struct lpm_tool_cmd), 0, sizeof(struct lpm_tool_cmd));

	// copy command and args
	ret = copy_from_user((void *)tool_cmd->cmd_data.raw_data, (void __user *)in, in_len);
	if (ret != 0) {
		lpm_log_err("copy msg to tool_cmd failed %d\n", ret);
		return -1;
	}

	if (!tool_cmd_format(tool_cmd, in_len)) {
		lpm_log_err("tool cmd format failed\n");
		return -1;
	}

	if (cmd_handler[tool_cmd->cmd_type] == NULL) {
		lpm_log_err("handler for cmd type(%u) not support\n", (uint32_t)tool_cmd->cmd_type);
		return -1;
	}

	return cmd_handler[(uint32_t)tool_cmd->cmd_type](dev_id, tool_cmd);
}

int32_t execute_quick_cmd(uint32_t dev_id, const struct lpm_tool_cmd *tool_cmd)
{
	int32_t ret = -1;
	const struct lpm_lptest_quick_cmd *quick_cmd =
		(const struct lpm_lptest_quick_cmd *)&tool_cmd->cmd_data.quick_cmd;
	switch (quick_cmd->cmd_type) {
	case LPTEST_QUICK_CMD_POWER_SAMPLING:
		ret = 0;
		break;
	case LPTEST_QUICK_CMD_SET_AIC_MAX_FREQ:
		if (quick_cmd->data_len == sizeof(uint32_t)) {
			uint32_t aic_max_freq = *(uint32_t*)quick_cmd->data;
			ret = lpm_turbo_set_aic_max_freq(dev_id, aic_max_freq);
		}
		break;
	default:
		lpm_log_err("execute quick cmd failed cmd_type=%u\n", quick_cmd->cmd_type);
		break;
	}
	return ret;
}

// send ipc to lpmcu, result should be ready once returned
STATIC int32_t lpm_send_lptest_ipc(uint32_t dev_id, const struct lpm_lptest_cmd *lptest_cmd)
{
	struct lpm_lptest_ipc_ack ack_data = {0};
	struct lpm_devmng_ipc_msg ipc_msg;
	int32_t ret;

	ipc_msg.send_type = LPM_IPC_SET_LP_TEST;
	ipc_msg.in        = (void *)lptest_cmd->cmd_name;
	ipc_msg.in_len    = LPTEST_CMD_NAME_LEN;
	ipc_msg.out       = (void *)&ack_data;
	ipc_msg.out_pos   = 0;
	ipc_msg.out_len   = (uint32_t)sizeof(ack_data);
	ret = lpm_ipc_send_sync_msg(dev_id, &ipc_msg);
	if (ret != 0) {
		lpm_log_err("send lptest ipc msg failed %d\n", ret);
		return ret;
	}

	lpm_log_info("lptest to lpmcu ipc result=%u\n", ack_data.result);
	return 0;
}

// process lptest cmd in buffer, save result in global buffer
STATIC int32_t execute_lptest_cmd(uint32_t dev_id, const struct lpm_tool_cmd *tool_cmd)
{
	int32_t ret;
	const struct lpm_lptest_cmd *lptest_cmd = &tool_cmd->cmd_data.lptest_cmd;

	ret = lpm_save_lptest_cmd_to_sharemem(dev_id, lptest_cmd);
	if (ret != 0) {
		lpm_log_err("save lp cmd to sharemem failed %d\n", ret);
		return ret;
	}

	// send ipc to lpmcu with command, result is ready after return
	ret = lpm_send_lptest_ipc(dev_id, lptest_cmd);
	if (ret != 0) {
		lpm_log_err("lptest ipc send failed %d\n", ret);
		return ret;
	}

	// save command result log in ddr share memory and update log length
	ret = lpm_read_lptest_result_from_sharemem(
		dev_id, &g_log_length[dev_id], g_log_buffer[dev_id], LPTEST_RESULT_BUFF_LEN);
	if (ret != 0) {
		lpm_log_err("read lptest result from sharemem failed %d\n", ret);
		return ret;
	}
	return 0;
}

// copy result from global buffer to user buffer, out_len return 0 on fail
STATIC int32_t copy_result_to_user(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	unsigned long cpy_len;
	uint32_t log_out_len;

	*out_len = 0;

	if (g_log_length[dev_id] == 0) {
		lpm_log_err("lptest result not ready\n");
		return -ENODATA;
	}

	log_out_len = min_t(uint32_t, g_log_length[dev_id], in_len);
	g_log_buffer[dev_id][log_out_len - 1] = 0; // avoid non-ended string
	cpy_len = copy_to_user((void __user *)in, g_log_buffer[dev_id], log_out_len);
	if (cpy_len != 0) {
		lpm_log_err("logout copy_to_tool_msg.out_buff failed, cpy_len=%lu\n", cpy_len);
		return -EIO;
	}

	*out_len = log_out_len;
	return 0;
}

STATIC int32_t quick_cmd_power_sampling(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	uint32_t i;
	unsigned long cpy_len;
	uint32_t offset = 0;
	uint32_t temperature = 0;
	if (in_len < sizeof(uint32_t) * 0x4) {
		return -1;
	}
	for (i = 0; i < 0x4; i++) {
		(void)lpm_get_temperature_from_sharemem(dev_id, (enum lpm_devmng_temperature_type)i, &temperature);
		cpy_len = copy_to_user((void *)((uintptr_t)in + (uintptr_t)offset), &temperature, sizeof(uint32_t));
		if (cpy_len != 0) {
			lpm_log_err("temprature copy to qucik tool failed, cpy_len=%lu\n", cpy_len);
			return -EIO;
		}
		offset += sizeof(uint32_t);
	}
	*out_len = offset;

	return lpm_read_power_sampling_from_sharemem(dev_id,
		(char *)((uintptr_t)in + (uintptr_t)offset), in_len - offset, out_len);
}

STATIC int32_t quick_cmd_get_result(const struct lpm_tool_cmd *tool_cmd,
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	int32_t ret = -1;
	const struct lpm_lptest_quick_cmd *quick_cmd =
		(const struct lpm_lptest_quick_cmd *)&tool_cmd->cmd_data.quick_cmd;
	switch (quick_cmd->cmd_type) {
	case LPTEST_QUICK_CMD_POWER_SAMPLING:
		ret = quick_cmd_power_sampling(dev_id, in, in_len, out_len);
		break;
	default:
		lpm_log_err("execute quick cmd failed cmd_type=%u\n", quick_cmd->cmd_type);
		break;
	}
	return ret;
}

STATIC int32_t lpm_dsmi_get_result(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	const struct lpm_tool_cmd *tool_cmd = &g_tool_cmd[dev_id];

	if ((in == NULL) || (in_len == 0)) {
		lpm_log_err("lptest out buff invalid\n");
		return -EINVAL;
	}
	if (tool_cmd->cmd_type == LPM_CMD_TYPE_QUICK) {
		return quick_cmd_get_result(tool_cmd, dev_id, in, in_len, out_len);
	} else {
		return copy_result_to_user(dev_id, in, in_len, out_len);
	}
	return 0;
}

STATIC int32_t lptest_ioctl_locked(struct file *file, unsigned int dev_id, unsigned long arg)
{
	int32_t ret;
	uint32_t out_len;
	struct lpm_lptest_msg tool_msg;

	(void)file;
	if (arg == 0UL) {
		lpm_log_err("lptest tool msg NULL\n");
		return -EINVAL;
	}

	ret = copy_from_user(&tool_msg, (void __user *)arg, sizeof(struct lpm_lptest_msg));
	if (ret != 0) {
		lpm_log_err("copy test msg to tool msg fail %d\n", ret);
		return -EIO;
	}

	ret = execute_tool_cmd(dev_id, (const char *)tool_msg.in_buff, tool_msg.in_buff_len);
	if (ret != 0) {
		lpm_log_err("lptest cmd execute fail %d\n", ret);
		return ret;
	}

	if ((g_tool_cmd[dev_id].cmd_type == LPM_CMD_TYPE_QUICK) &&
		(g_tool_cmd[dev_id].cmd_data.quick_cmd.res_ignore == 1)) {
		return 0;
	} else {
		return lpm_dsmi_get_result(dev_id, tool_msg.out_buff, tool_msg.out_buff_len, &out_len);
	}
}

// lptest ioctl handler, dev_id in cmd, lptest tool msg in arg
STATIC long lptest_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int32_t ret;

	(void)file;
	if (cmd >= lpm_common_get_dev_num()) {
		lpm_log_err("lptest ioctl cmd=%u dev_num=%u\n", cmd, lpm_common_get_dev_num());
		return -EINVAL;
	}
	ret = down_timeout(&g_lptest_set_sem[cmd], LPM_LPTEST_TIMEOUT);
	if (ret != 0) {
		lpm_log_err("wait lptest free timeout %d\n", ret);
		return ret;
	}

	ret = lptest_ioctl_locked(file, cmd, arg);
	up(&g_lptest_set_sem[cmd]);
	return (long)ret;
}

STATIC int32_t lptest_open(struct inode *inode, struct file *file)
{
	(void)inode;
	(void)file;
	return 0;
}

STATIC int32_t lptest_release(struct inode *inode, struct file *file)
{
	(void)inode;
	(void)file;
	return 0;
}

STATIC const struct file_operations g_lptest_fops = {
	.owner = THIS_MODULE,
	.open = lptest_open,
	.release = lptest_release,
#ifdef CONFIG_COMPAT
	.compat_ioctl = lptest_ioctl,
#endif
	.unlocked_ioctl = lptest_ioctl,
};

int32_t lpm_dsmi_set_lptest(uint32_t dev_id, const char *in, uint32_t in_len)
{
	int32_t ret;

	if ((in == NULL) || (in_len == 0)) {
		lpm_log_err("lptest set param invalid\n");
		return -EINVAL;
	}
	if (dev_id >= lpm_common_get_dev_num()) {
		lpm_log_err("dsmi set lptest dev_id=%u dev_num=%u\n", dev_id, lpm_common_get_dev_num());
		return -EINVAL;
	}

	ret = down_timeout(&g_lptest_set_sem[dev_id], LPM_LPTEST_TIMEOUT);
	if (ret != 0) {
		lpm_log_err("wait lptest free failed %d\n", ret);
		return ret;
	}

	ret = execute_tool_cmd(dev_id, in, in_len);
	if (ret != 0) {
		lpm_log_err("lptest command execute failed %d\n", ret);
		up(&g_lptest_set_sem[dev_id]);
		return ret;
	}

	if ((g_tool_cmd[dev_id].cmd_type == LPM_CMD_TYPE_QUICK) &&
		(g_tool_cmd[dev_id].cmd_data.quick_cmd.res_ignore == 1)) {
		up(&g_lptest_set_sem[dev_id]);
	} else {
		up(&g_lptest_get_sem[dev_id]);
	}
	return 0;
}

int32_t lpm_dsmi_get_lptest(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	int32_t ret;

	if ((in == NULL) || (in_len == 0) || (out_len == NULL)) {
		lpm_log_err("lptest get param invalid\n");
		return -EINVAL;
	}
	if (dev_id >= lpm_common_get_dev_num()) {
		lpm_log_err("dsmi get lptest dev_id=%u dev_num=%u\n", dev_id, lpm_common_get_dev_num());
		return -EINVAL;
	}

	if (down_trylock(&g_lptest_get_sem[dev_id])) {
		lpm_log_err("lptest result not ready\n");
		*out_len = 0;
		return -ENODATA;
	}

	ret = lpm_dsmi_get_result(dev_id, in, in_len, out_len);
	up(&g_lptest_set_sem[dev_id]);
	return ret;
}

STATIC int32_t lptest_cdev_init(void)
{
	uint32_t i;
	int32_t ret;
	dev_t cdev_id = 0;
	struct device *dev = NULL;

	ret = alloc_chrdev_region(&cdev_id, 0, 1, LP_NAME);
	if (ret < 0) {
		lpm_log_err("alloc %s chrdev err\n", LP_NAME);
		return -ENODEV;
	}

	cdev_init(&g_lptest_dev.cdev, &g_lptest_fops);
	g_lptest_dev.cdev.owner = THIS_MODULE;

	ret = cdev_add(&g_lptest_dev.cdev, cdev_id, 1);
	if (ret != 0) {
		lpm_log_err("cdev_add 0x%X err\n", cdev_id);
		goto cdev_add_err;
	}

	g_lptest_dev.myclass = class_create(THIS_MODULE, LP_NAME);
	if (IS_ERR(g_lptest_dev.myclass)) {
		ret = -1;
		lpm_log_err("class_create %s err\n", LP_NAME);
		goto class_create_err;
	}

	dev = device_create(g_lptest_dev.myclass, NULL, cdev_id, NULL, LP_NAME);
	if (IS_ERR(dev)) {
		ret = -ENXIO;
		lpm_log_err("device_create %s err\n", LP_NAME);
		goto device_create_err;
	}
	g_lptest_dev.dev = dev;
	g_lptest_dev.dev_id = cdev_id;
	for (i = 0; i < lpm_common_get_dev_num(); i++) {
		sema_init(&g_lptest_set_sem[i], 1);
		sema_init(&g_lptest_get_sem[i], 0);
	}
	lpm_log_info("dev %s create ok\n", LP_NAME);
	return 0;

device_create_err:
	class_destroy(g_lptest_dev.myclass);
class_create_err:
	cdev_del(&g_lptest_dev.cdev);
cdev_add_err:
	unregister_chrdev_region(cdev_id, 1);
	return ret;
}

STATIC void lptest_cdev_remove(void)
{
	device_destroy(g_lptest_dev.myclass, g_lptest_dev.dev_id);
	class_destroy(g_lptest_dev.myclass);
	cdev_del(&g_lptest_dev.cdev);
	unregister_chrdev_region(g_lptest_dev.dev_id, 1);
}

int32_t lpm_lptest_init(uint64_t *param, uint32_t param_num)
{
	int32_t ret;

	ret = lptest_cdev_init();
	if (ret != 0) {
		lpm_log_err("lptest init fail %d\n", ret);
		return ret;
	}

	(void)param;
	(void)param_num;
	lpm_log_info("lptest init success\n");
	lpm_sr_test_init();
	return 0;
}

int32_t lpm_lptest_exit(uint64_t *param, uint32_t param_num)
{
	lptest_cdev_remove();

	(void)param;
	(void)param_num;
	lpm_log_info("lptest remove success\n");
	lpm_sr_test_exit();
	return 0;
}

#ifdef LPM_SUSPEND
static int32_t g_lpm_suspend_inject_type = -1;
static int32_t g_lpm_suspend_inject_stage = -1;
// process suspend cmd, save result in global buffer
STATIC int32_t execute_suspend_cmd(uint32_t dev_id, const struct lpm_tool_cmd *tool_cmd)
{
	int32_t ret;
	int32_t inject_stage = LPM_TEST_SUSPEND;
	int32_t inject_type = (int32_t)tool_cmd->cmd_data.lptest_cmd.args[0];

	(void)dev_id;
	if (tool_cmd->cmd_data.lptest_cmd.num_args == 0x2)
		inject_stage = (int32_t)tool_cmd->cmd_data.lptest_cmd.args[1];

	if ((inject_type > LPM_TEST_TIMEOUT) || (inject_stage >= LPM_TEST_SR_MAX)) {
		g_lpm_suspend_inject_type = -1;
		g_lpm_suspend_inject_stage = -1;
		ret = sprintf_s(g_log_buffer[dev_id], LPTEST_RESULT_BUFF_LEN,
			"suspend inject type=%d stage=%d cleared\n", inject_type, inject_stage);
	} else {
		g_lpm_suspend_inject_type = inject_type;
		g_lpm_suspend_inject_stage = inject_stage;
		ret = sprintf_s(g_log_buffer[dev_id], LPTEST_RESULT_BUFF_LEN,
			"suspend inject type=%d stage=%d update\n", inject_type, inject_stage);
	}

	// update result log, ret + 1 always >= 0 for ret >= -1 always
	g_log_length[dev_id] = ret + 1;
	return 0;
}

STATIC int32_t lpm_suspend_test_ops(int32_t current_stage)
{
	const char *stage[LPM_TEST_SR_MAX] = {
		"LPM_SUSPEND_PREPARE",
		"LPM_SUSPEND",
		"LPM_SUSPEND_LATE",
		"LPM_SUSPEND_NO_IRQ",
		"LPM_RESUME_NO_IRQ",
		"LPM_RESUME_EARLY",
		"LPM_RESUME",
		"LPM_POST_SUSPEND"
	};

	if (current_stage == g_lpm_suspend_inject_stage) {
		switch (g_lpm_suspend_inject_type) {
		case LPM_TEST_FAIL:
			lpm_log_err("lpm_sr_test [%s] inject fail\n", stage[current_stage]);
			return -1;
		case LPM_TEST_PANIC:
			lpm_log_err("lpm_sr_test [%s] inject kernel panic!!!\n", stage[current_stage]);
			BUG_ON(1);
			break;
		case LPM_TEST_TIMEOUT:
			lpm_log_err("lpm_sr_test [%s] inject timeout 20s\n", stage[current_stage]);
			usleep_range(LPTEST_SR_TIMEOUT_MIN, LPTEST_SR_TIMEOUT_MAX);
			break;
		default:
			break;
		}
	}

	lpm_log_info("lpm_sr_test [%s] succ\n", stage[current_stage]);
	return 0;
}

STATIC int32_t lpm_test_suspend_prepare(void)
{
	return lpm_suspend_test_ops(LPM_TEST_SUSPEND_PREPARE);
}

STATIC int32_t lpm_test_suspend(struct device *dev)
{
	(void)dev;
	return lpm_suspend_test_ops(LPM_TEST_SUSPEND);
}

STATIC int32_t lpm_test_suspend_late(struct device *dev)
{
	(void)dev;
	return lpm_suspend_test_ops(LPM_TEST_SUSPEND_LATE);
}

STATIC int32_t lpm_test_suspend_noirq(struct device *dev)
{
	(void)dev;
	return lpm_suspend_test_ops(LPM_TEST_SUSPEND_NO_IRQ);
}

STATIC int32_t lpm_test_resume_noirq(struct device *dev)
{
	(void)dev;
	return lpm_suspend_test_ops(LPM_TEST_RESUME_NO_IRQ);
}

STATIC int32_t lpm_test_resume_early(struct device *dev)
{
	(void)dev;
	return lpm_suspend_test_ops(LPM_TEST_RESUME_EARLY);
}

STATIC int32_t lpm_test_resume(struct device *dev)
{
	(void)dev;
	return lpm_suspend_test_ops(LPM_TEST_RESUME);
}

STATIC int32_t lpm_test_post_suspend(void)
{
	return lpm_suspend_test_ops(LPM_TEST_POST_SUSPEND);
}

static const struct dev_pm_ops g_lpm_suspend_test_pm_ops = {
	.suspend = lpm_test_suspend,
	.suspend_late = lpm_test_suspend_late,
	.suspend_noirq = lpm_test_suspend_noirq,
	.resume_noirq = lpm_test_resume_noirq,
	.resume_early = lpm_test_resume_early,
	.resume = lpm_test_resume,
};

STATIC int32_t lpm_test_notify_nb(struct notifier_block *nb, unsigned long mode, void *unused)
{
	int32_t ret;

	(void)nb;
	(void)unused;

	switch (mode) {
	case PM_SUSPEND_PREPARE:
		ret = lpm_test_suspend_prepare();
		break;
	case PM_POST_SUSPEND:
		ret = lpm_test_post_suspend();
		break;
	default:
		// not support
		ret = -1;
		break;
	}

	return (ret != 0) ? NOTIFY_BAD : NOTIFY_OK;
}

static struct notifier_block lpm_test_pm_nb = {
	.notifier_call = lpm_test_notify_nb,
	.priority = 1,
};

STATIC int32_t lpm_suspend_test_probe(struct platform_device *pdev)
{
	(void)pdev;
	return register_pm_notifier(&lpm_test_pm_nb);
}

STATIC int32_t lpm_suspend_test_remove(struct platform_device *pdev)
{
	(void)pdev;
	return unregister_pm_notifier(&lpm_test_pm_nb);
}

static const struct of_device_id lpm_suspend_test_of_match[] = {
	{ .compatible = "hisi,lpm_sr_test"},
	{ }
};
MODULE_DEVICE_TABLE(of, lpm_suspend_test_of_match);

static struct platform_device *lpm_sr_test_device = NULL;
static struct platform_driver lpm_suspend_test_driver = {
	.probe = lpm_suspend_test_probe,
	.remove = lpm_suspend_test_remove,
	.driver = {
		.name           = "lpm_sr_test",
		.pm             = &g_lpm_suspend_test_pm_ops,
		.of_match_table = of_match_ptr(lpm_suspend_test_of_match),
	},
};

STATIC int32_t lpm_sr_test_init(void)
{
	int32_t err;
	struct platform_device *pdev = NULL;

	err = platform_driver_register(&lpm_suspend_test_driver);
	if (err) {
		lpm_log_err("lpm_sr_test driver register failed\n");
		return err;
	}

	pdev = platform_device_alloc("lpm_sr_test", -1);
	if (!pdev) {
		lpm_log_err("lpm_sr_test Device allocation failed\n");
		err = -ENOMEM;
		goto err_unregister_driver;
	}

	err = platform_device_add(pdev);
	if (err) {
		lpm_log_err("lpm_sr_test Device add failed\n");
		goto err_free_device;
	}

	lpm_sr_test_device = pdev;
	return 0;

err_free_device:
	platform_device_put(pdev);
err_unregister_driver:
	platform_driver_unregister(&lpm_suspend_test_driver);
	return err;
}

STATIC void lpm_sr_test_exit(void)
{
	platform_device_unregister(lpm_sr_test_device);
	platform_driver_unregister(&lpm_suspend_test_driver);
}

#else

STATIC int32_t execute_suspend_cmd(uint32_t dev_id, const struct lpm_tool_cmd *tool_cmd)
{
	(void)dev_id;
	(void)tool_cmd;
	return -EINVAL;
}

STATIC int32_t lpm_sr_test_init(void)
{
	return 0;
}

STATIC void lpm_sr_test_exit(void)
{
}

#endif // end of LPM_SUSPEND

#endif
