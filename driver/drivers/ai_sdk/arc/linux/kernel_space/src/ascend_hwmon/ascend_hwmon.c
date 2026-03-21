/*
 * Ascend 310B SoC temperature hwmon driver
 *
 * Reads temperature from LPM3 shared memory and exposes it
 * via the Linux hwmon framework so that tools like btop/htop
 * can display CPU/SoC temperatures.
 *
 * Shared memory layout (struct dmanage_temp_share_mem):
 *   offset 0: s8 cluster_temp
 *   offset 1: s8 peri_temp
 *   offset 2: s8 aicore0_temp
 *   offset 3: s8 aicore1_temp
 *   offset 9: s8 soc_max_temp
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/hwmon.h>
#include <linux/io.h>

#define TSENSOR_SHM_PHYS	0x000A20000ULL
#define TSENSOR_SHM_SIZE	0x1000

#define OFF_CLUSTER_TEMP	0
#define OFF_PERI_TEMP		1
#define OFF_AICORE0_TEMP	2
#define OFF_AICORE1_TEMP	3
#define OFF_SOC_MAX_TEMP	9

struct ascend_hwmon_data {
	void __iomem *shm_base;
	struct platform_device *pdev;
};

static int ascend_hwmon_read(struct device *dev, enum hwmon_sensor_types type,
			     u32 attr, int channel, long *val)
{
	struct ascend_hwmon_data *data = dev_get_drvdata(dev);
	s8 temp;

	if (type != hwmon_temp || attr != hwmon_temp_input)
		return -EOPNOTSUPP;

	switch (channel) {
	case 0:
		temp = readb(data->shm_base + OFF_CLUSTER_TEMP);
		break;
	case 1:
		temp = readb(data->shm_base + OFF_PERI_TEMP);
		break;
	case 2:
		temp = readb(data->shm_base + OFF_AICORE0_TEMP);
		break;
	case 3:
		temp = readb(data->shm_base + OFF_AICORE1_TEMP);
		break;
	case 4:
		temp = readb(data->shm_base + OFF_SOC_MAX_TEMP);
		break;
	default:
		return -EOPNOTSUPP;
	}

	/* hwmon expects millidegrees Celsius */
	*val = (long)temp * 1000;
	return 0;
}

static umode_t ascend_hwmon_is_visible(const void *data,
				       enum hwmon_sensor_types type,
				       u32 attr, int channel)
{
	if (type != hwmon_temp || channel >= 5)
		return 0;
	if (attr == hwmon_temp_input || attr == hwmon_temp_label)
		return 0444;
	return 0;
}

/*
 * btop auto-detects CPU sensor by matching labels:
 *   "Package id", "Tdie", "SoC Temperature"
 * Use "SoC Temperature" for channel 0 so btop picks it up as CPU temp.
 */
static const char * const ascend_hwmon_temp_labels[] = {
	"SoC Temperature",
	"Peripherals",
	"AI Core 0",
	"AI Core 1",
	"SoC Max",
};

static int ascend_hwmon_read_string(struct device *dev,
				    enum hwmon_sensor_types type,
				    u32 attr, int channel,
				    const char **str)
{
	if (type != hwmon_temp || attr != hwmon_temp_label || channel >= 5)
		return -EOPNOTSUPP;

	*str = ascend_hwmon_temp_labels[channel];
	return 0;
}

static const struct hwmon_channel_info * const ascend_hwmon_info[] = {
	HWMON_CHANNEL_INFO(temp,
			   HWMON_T_INPUT | HWMON_T_LABEL,
			   HWMON_T_INPUT | HWMON_T_LABEL,
			   HWMON_T_INPUT | HWMON_T_LABEL,
			   HWMON_T_INPUT | HWMON_T_LABEL,
			   HWMON_T_INPUT | HWMON_T_LABEL),
	NULL
};

static const struct hwmon_ops ascend_hwmon_ops = {
	.is_visible = ascend_hwmon_is_visible,
	.read = ascend_hwmon_read,
	.read_string = ascend_hwmon_read_string,
};

static const struct hwmon_chip_info ascend_hwmon_chip_info = {
	.ops = &ascend_hwmon_ops,
	.info = ascend_hwmon_info,
};

static struct ascend_hwmon_data *g_hwmon_data;
static struct device *g_hwmon_dev;

static int __init ascend_hwmon_init(void)
{
	struct platform_device *pdev;
	struct device *hwmon_dev;

	g_hwmon_data = kzalloc(sizeof(*g_hwmon_data), GFP_KERNEL);
	if (!g_hwmon_data)
		return -ENOMEM;

	g_hwmon_data->shm_base = ioremap(TSENSOR_SHM_PHYS, TSENSOR_SHM_SIZE);
	if (!g_hwmon_data->shm_base) {
		pr_err("ascend_hwmon: failed to ioremap tsensor shared memory\n");
		kfree(g_hwmon_data);
		return -ENOMEM;
	}

	/* hwmon_device_register_with_info requires a non-NULL parent device */
	pdev = platform_device_register_simple("ascend_hwmon", -1, NULL, 0);
	if (IS_ERR(pdev)) {
		pr_err("ascend_hwmon: failed to register platform device\n");
		iounmap(g_hwmon_data->shm_base);
		kfree(g_hwmon_data);
		return PTR_ERR(pdev);
	}
	g_hwmon_data->pdev = pdev;

	hwmon_dev = hwmon_device_register_with_info(&pdev->dev, "ascend310b",
						    g_hwmon_data,
						    &ascend_hwmon_chip_info,
						    NULL);
	if (IS_ERR(hwmon_dev)) {
		pr_err("ascend_hwmon: failed to register hwmon device\n");
		platform_device_unregister(pdev);
		iounmap(g_hwmon_data->shm_base);
		kfree(g_hwmon_data);
		return PTR_ERR(hwmon_dev);
	}

	g_hwmon_dev = hwmon_dev;
	pr_info("ascend_hwmon: registered (tsensor shm @ 0x%llx)\n",
		(unsigned long long)TSENSOR_SHM_PHYS);
	return 0;
}

static void __exit ascend_hwmon_exit(void)
{
	if (g_hwmon_dev)
		hwmon_device_unregister(g_hwmon_dev);
	if (g_hwmon_data) {
		if (g_hwmon_data->pdev)
			platform_device_unregister(g_hwmon_data->pdev);
		if (g_hwmon_data->shm_base)
			iounmap(g_hwmon_data->shm_base);
		kfree(g_hwmon_data);
	}
}

module_init(ascend_hwmon_init);
module_exit(ascend_hwmon_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Ascend 310B SoC temperature hwmon bridge");
