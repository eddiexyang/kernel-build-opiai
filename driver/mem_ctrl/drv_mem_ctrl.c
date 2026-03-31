// SPDX-License-Identifier: GPL-2.0
#include <asm/io.h>
#include <asm/pgtable-prot.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#define MEM_CTRL_TAG "mem_ctrl"
#define MEM_CTRL_DEV_NAME "dev_mem_ctrl"
#define MEM_CTRL_COMPATIBLE "hisilicon,hi1910B-evb"
#define MEM_CTRL_BOARDID_PROP "hisi,boardid"

#define MEM_CTRL_BOOT_SEL_BASE 0xC0120000ULL
#define MEM_CTRL_BOOT_SEL_SIZE 0xF000U
#define MEM_CTRL_BOOT_SEL_REG  0x8000U
#define MEM_CTRL_BOOT_SEL_MASK 0x1C0U
#define MEM_CTRL_BOOT_SEL_A    0x100U
#define MEM_CTRL_BOOT_SEL_B    0x140U
#define MEM_CTRL_BOOT_SEL_C    0x0C0U

#define MEM_CTRL_RESET_COUNT_BASE 0xC014EB08ULL
#define MEM_CTRL_RESET_COUNT_SIZE 0x4U
#define MEM_CTRL_RESET_COUNT_MAX  7U

#define MEM_CTRL_MAC0_BASE    0xA7280000ULL
#define MEM_CTRL_MAC1_BASE    0xA7290000ULL
#define MEM_CTRL_MAC_WINDOW   0x10000U
#define MEM_CTRL_TX_ERR_OFF   0x98U
#define MEM_CTRL_RX_ERR_OFF   0x1F8U

#define MEM_CTRL_CMD_GET_BOOT_SEL    _IOR('M', 0x00, u32)
#define MEM_CTRL_CMD_SET_RESET_COUNT _IOW('M', 0x01, u32)
#define MEM_CTRL_CMD_GET_BOARD_ID    _IOR('M', 0x02, u32)
#define MEM_CTRL_CMD_GET_MAC0_RX_ERR _IOR('M', 0x03, u64)
/* IDA compare chain resolves the in-between branch to 0x80044D04. */
#define MEM_CTRL_CMD_GET_MAC0_TX_ERR _IOR('M', 0x04, u64)
#define MEM_CTRL_CMD_GET_MAC1_RX_ERR _IOR('M', 0x05, u64)
#define MEM_CTRL_CMD_GET_MAC1_TX_ERR _IOR('M', 0x06, u64)

#define MEM_CTRL_LOG_ERR(fn, line, fmt, ...) \
	printk(KERN_ERR MEM_CTRL_TAG ":%s:%d " fmt, (fn), (line), ##__VA_ARGS__)
#define MEM_CTRL_LOG_INFO(fn, line, fmt, ...) \
	printk(KERN_INFO MEM_CTRL_TAG ":%s:%d " fmt, (fn), (line), ##__VA_ARGS__)

struct mem_ctrl_dev {
	u32 major;
	u32 minor;
	struct cdev cdev;
	struct class *dev_class;
};

static struct mem_ctrl_dev *g_mem_ctrl_dev;

static int mem_ctrl_release(struct inode *inode, struct file *file)
{
	if (!inode || !file) {
		MEM_CTRL_LOG_ERR("mem_ctrl_release", 413, "invalid release context\n");
		return -EINVAL;
	}

	file->private_data = NULL;
	return 0;
}

static int mem_ctrl_open(struct inode *inode, struct file *file)
{
	if (!inode || !file) {
		MEM_CTRL_LOG_ERR("mem_ctrl_open", 46, "invalid open context\n");
		return -EINVAL;
	}

	if (g_mem_ctrl_dev)
		file->private_data = g_mem_ctrl_dev;

	return 0;
}

static void __iomem *mem_ctrl_ioremap(phys_addr_t phys, size_t size)
{
	pgprot_t prot = __pgprot(arm64_use_ng_mappings ?
				 PROT_NORMAL_NC : PROT_DEVICE_nGnRE);

	return __ioremap_prot(phys, size, prot);
}

static int mem_ctrl_get_boot_sel(unsigned long arg)
{
	void __iomem *base;
	u32 reg;
	u32 boot_sel;

	if (!arg) {
		MEM_CTRL_LOG_ERR("mem_ctrl_get_boot_sel", 145, "user pointer is null\n");
		return -EINVAL;
	}

	base = mem_ctrl_ioremap(MEM_CTRL_BOOT_SEL_BASE, MEM_CTRL_BOOT_SEL_SIZE);
	if (!base) {
		MEM_CTRL_LOG_ERR("mem_ctrl_get_boot_sel", 152, "ioremap failed\n");
		return -ENOMEM;
	}

	reg = readl_relaxed((u8 __iomem *)base + MEM_CTRL_BOOT_SEL_REG);
	dma_rmb();
	iounmap(base);

	switch (reg & MEM_CTRL_BOOT_SEL_MASK) {
	case MEM_CTRL_BOOT_SEL_A:
		boot_sel = 0;
		break;
	case MEM_CTRL_BOOT_SEL_B:
		boot_sel = 1;
		break;
	case MEM_CTRL_BOOT_SEL_C:
		boot_sel = 2;
		break;
	default:
		MEM_CTRL_LOG_ERR("mem_ctrl_get_boot_sel", 167,
				 "unsupported boot select value %#x\n",
				 reg & MEM_CTRL_BOOT_SEL_MASK);
		return -EIO;
	}

	if (copy_to_user((void __user *)arg, &boot_sel, sizeof(boot_sel))) {
		MEM_CTRL_LOG_ERR("mem_ctrl_get_boot_sel", 172, "copy_to_user failed\n");
		return -EINVAL;
	}

	MEM_CTRL_LOG_INFO("mem_ctrl_get_boot_sel", 176, "boot_sel=%u\n", boot_sel);
	return 0;
}

static int mem_ctrl_set_reset_count(unsigned long arg)
{
	void __iomem *base;
	u32 reset_count = 0;

	if (!arg) {
		MEM_CTRL_LOG_ERR("mem_ctrl_set_reset_count", 187, "user pointer is null\n");
		return -EINVAL;
	}

	if (copy_from_user(&reset_count, (void __user *)arg, sizeof(reset_count))) {
		MEM_CTRL_LOG_ERR("mem_ctrl_set_reset_count", 192,
				 "copy_from_user failed\n");
		return -EINVAL;
	}

	if (reset_count > MEM_CTRL_RESET_COUNT_MAX) {
		MEM_CTRL_LOG_ERR("mem_ctrl_set_reset_count", 197,
				 "reset_count=%u exceeds max\n", reset_count);
		return -EINVAL;
	}

	base = mem_ctrl_ioremap(MEM_CTRL_RESET_COUNT_BASE, MEM_CTRL_RESET_COUNT_SIZE);
	if (!base) {
		MEM_CTRL_LOG_ERR("mem_ctrl_set_reset_count", 204, "ioremap failed\n");
		return -ENOMEM;
	}

	dma_wmb();
	writel_relaxed(reset_count, base);
	iounmap(base);

	MEM_CTRL_LOG_INFO("mem_ctrl_set_reset_count", 209,
			 "reset_count=%u\n", reset_count);
	return 0;
}

static int mem_ctrl_get_board_id(unsigned long arg)
{
	struct device_node *np;
	u32 boardid[6] = { 0 };
	u32 folded;
	int ret;

	if (!arg) {
		MEM_CTRL_LOG_ERR("mem_ctrl_get_board_id", 222, "user pointer is null\n");
		return -EINVAL;
	}

	np = of_find_compatible_node(NULL, NULL, MEM_CTRL_COMPATIBLE);
	if (!np) {
		MEM_CTRL_LOG_ERR("mem_ctrl_get_board_id", 228,
				 "compatible node not found\n");
		return -ENOENT;
	}

	ret = of_property_read_variable_u32_array(np, MEM_CTRL_BOARDID_PROP,
						  boardid, ARRAY_SIZE(boardid), 0);
	if (ret < 0) {
		MEM_CTRL_LOG_ERR("mem_ctrl_get_board_id", 233,
				 "boardid property read failed: %d\n", ret);
		return -ENOENT;
	}

	folded = boardid[0] * 100000U + boardid[1] * 10000U +
		 boardid[2] * 1000U + boardid[3] * 100U +
		 boardid[4] * 10U + boardid[5];

	if (copy_to_user((void __user *)arg, &folded, sizeof(folded))) {
		MEM_CTRL_LOG_ERR("mem_ctrl_get_board_id", 240,
				 "copy_to_user failed\n");
		return -EINVAL;
	}

	MEM_CTRL_LOG_INFO("mem_ctrl_get_board_id", 244, "board_id=%u\n", folded);
	return 0;
}

static int mem_ctrl_get_mac_err(unsigned long arg, phys_addr_t base_addr,
				u32 reg_off, const char *func_name,
				u32 null_line, u32 map_line, u32 copy_line)
{
	void __iomem *base;
	u64 value;

	if (!arg) {
		MEM_CTRL_LOG_ERR(func_name, null_line, "user pointer is null\n");
		return -EINVAL;
	}

	base = mem_ctrl_ioremap(base_addr, MEM_CTRL_MAC_WINDOW);
	if (!base) {
		MEM_CTRL_LOG_ERR(func_name, map_line, "ioremap failed\n");
		return -ENOMEM;
	}

	value = readq_relaxed((u8 __iomem *)base + reg_off);
	dma_rmb();
	iounmap(base);

	if (copy_to_user((void __user *)arg, &value, sizeof(value))) {
		MEM_CTRL_LOG_ERR(func_name, copy_line, "copy_to_user failed\n");
		return -EINVAL;
	}

	return 0;
}

static long mem_ctrl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	if (!file) {
		MEM_CTRL_LOG_ERR("mem_ctrl_ioctl", 397, "file is null\n");
		return -EINVAL;
	}

	switch (cmd) {
	case MEM_CTRL_CMD_GET_BOOT_SEL:
		return mem_ctrl_get_boot_sel(arg);
	case MEM_CTRL_CMD_SET_RESET_COUNT:
		return mem_ctrl_set_reset_count(arg);
	case MEM_CTRL_CMD_GET_BOARD_ID:
		return mem_ctrl_get_board_id(arg);
	case MEM_CTRL_CMD_GET_MAC0_RX_ERR:
		return mem_ctrl_get_mac_err(arg, MEM_CTRL_MAC0_BASE,
					    MEM_CTRL_RX_ERR_OFF,
					    "mem_ctrl_get_mac0_rx_err",
					    255, 262, 271);
	case MEM_CTRL_CMD_GET_MAC0_TX_ERR:
		return mem_ctrl_get_mac_err(arg, MEM_CTRL_MAC0_BASE,
					    MEM_CTRL_TX_ERR_OFF,
					    "mem_ctrl_get_mac0_tx_err",
					    285, 292, 301);
	case MEM_CTRL_CMD_GET_MAC1_RX_ERR:
		return mem_ctrl_get_mac_err(arg, MEM_CTRL_MAC1_BASE,
					    MEM_CTRL_RX_ERR_OFF,
					    "mem_ctrl_get_mac1_rx_err",
					    315, 322, 331);
	case MEM_CTRL_CMD_GET_MAC1_TX_ERR:
		return mem_ctrl_get_mac_err(arg, MEM_CTRL_MAC1_BASE,
					    MEM_CTRL_TX_ERR_OFF,
					    "mem_ctrl_get_mac1_tx_err",
					    345, 352, 361);
	default:
		MEM_CTRL_LOG_ERR("mem_ctrl_ioctl", 406,
				 "unsupported ioctl %#x\n", cmd);
		return -EINVAL;
	}
}

static const struct file_operations g_mem_ctrl_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = mem_ctrl_ioctl,
	.open = mem_ctrl_open,
	.release = mem_ctrl_release,
};

static int __init mem_ctrl_drv_init(void)
{
	struct class *dev_class;
	struct device *dev;
	dev_t devt;
	int ret;

	g_mem_ctrl_dev = kzalloc(sizeof(*g_mem_ctrl_dev), GFP_KERNEL);
	if (!g_mem_ctrl_dev) {
		MEM_CTRL_LOG_ERR("mem_ctrl_drv_init", 449, "device alloc failed\n");
		return -ENOMEM;
	}

	g_mem_ctrl_dev->dev_class = NULL;

	ret = alloc_chrdev_region(&devt, 0, 1, MEM_CTRL_DEV_NAME);
	if (ret < 0) {
		MEM_CTRL_LOG_ERR("init_mem_ctrl_dev", 77,
				 "alloc_chrdev_region failed: %d\n", ret);
		goto err_free;
	}

	dev_class = class_create(THIS_MODULE, MEM_CTRL_DEV_NAME);
	if (IS_ERR(dev_class)) {
		ret = PTR_ERR(dev_class);
		MEM_CTRL_LOG_ERR("init_mem_ctrl_dev", 83,
				 "class_create failed: %d\n", ret);
		goto err_unregister;
	}

	dev = device_create(dev_class, NULL, devt, NULL, MEM_CTRL_DEV_NAME);
	if (IS_ERR(dev)) {
		ret = PTR_ERR(dev);
		MEM_CTRL_LOG_ERR("init_mem_ctrl_dev", 90,
				 "device_create failed: %d\n", ret);
		goto err_class;
	}

	cdev_init(&g_mem_ctrl_dev->cdev, &g_mem_ctrl_fops);
	g_mem_ctrl_dev->cdev.owner = THIS_MODULE;

	ret = cdev_add(&g_mem_ctrl_dev->cdev, devt, 1);
	if (ret) {
		MEM_CTRL_LOG_ERR("init_mem_ctrl_dev", 99,
				 "cdev_add failed: %d\n", ret);
		device_destroy(dev_class, devt);
		goto err_class;
	}

	g_mem_ctrl_dev->dev_class = dev_class;
	g_mem_ctrl_dev->major = MAJOR(devt);
	g_mem_ctrl_dev->minor = MINOR(devt);

	MEM_CTRL_LOG_INFO("mem_ctrl_drv_init", 461,
			 "registered major=%u minor=%u\n",
			 g_mem_ctrl_dev->major, g_mem_ctrl_dev->minor);
	return 0;

err_class:
	class_destroy(dev_class);
err_unregister:
	unregister_chrdev_region(devt, 1);
err_free:
	MEM_CTRL_LOG_ERR("mem_ctrl_drv_init", 457, "init failed: %d\n", ret);
	kfree(g_mem_ctrl_dev);
	g_mem_ctrl_dev = NULL;
	return ret;
}

static void __exit mem_ctrl_drv_exit(void)
{
	dev_t devt;

	if (g_mem_ctrl_dev) {
		if (g_mem_ctrl_dev->dev_class) {
			devt = MKDEV(g_mem_ctrl_dev->major, g_mem_ctrl_dev->minor);
			cdev_del(&g_mem_ctrl_dev->cdev);
			device_destroy(g_mem_ctrl_dev->dev_class, devt);
			class_destroy(g_mem_ctrl_dev->dev_class);
			unregister_chrdev_region(devt, 1);
		} else {
			MEM_CTRL_LOG_ERR("cleanup_dev", 124,
					 "device class missing during cleanup\n");
		}

		kfree(g_mem_ctrl_dev);
		g_mem_ctrl_dev = NULL;
	}

	MEM_CTRL_LOG_INFO("mem_ctrl_drv_exit", 438, "driver exit\n");
}

module_init(mem_ctrl_drv_init);
module_exit(mem_ctrl_drv_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ascend drv_mem_ctrl device driver");
MODULE_AUTHOR("OpenAI");
