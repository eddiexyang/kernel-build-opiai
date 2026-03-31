/*
 * Ascend 310B SoC temperature hwmon driver
 *
 * Reads temperature from LPM3 shared memory and exposes it
 * via the Linux hwmon framework so that tools like btop/htop
 * can display CPU/SoC temperatures.
 *
 * LPM shared memory layout (V2, for 310B / MINIV2):
 *   Base addr: 0xA00000
 *   +0x20000 : DEVMNG region (100K)
 *     DEVMNG has a self-describing header:
 *       magic (4B) + version (4B) + entry_num (4B)
 *       entry[] array: { type(4B), offset(4B), len(4B) } * entry_num
 *       crc (4B)
 *     entry[0] = temperature, each value is uint16_t
 *       index 0: soc_max_temp
 *       index 1: aic_max_temp
 *       index 2: hbm_max_temp
 *       index 3: totem_max_temp
 *       index 4+: other temps
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/hwmon.h>
#include <linux/io.h>

/* LPM shared memory base address for Ascend 310B (MINIV3 / Milan) */
#define LPM_SHM_PHYS		0x3100000ULL
#define LPM_SHM_SIZE		0x200000ULL	/* 2M total */

/* DEVMNG region starts after 128K log buffer */
#define DEVMNG_OFFSET		0x20000ULL

/* Header magic: "LPSM" */
#define LPM_SHAREMEM_MAGIC	0x4C50534DU

/* DEVMNG entry types */
#define ENTRY_TYPE_TEMP		0

/* Temperature indices (enum lpm_devmng_temperature_type) */
#define TEMP_SOC_MAX		0
#define TEMP_AIC_MAX		1
#define TEMP_HBM_MAX		2
#define TEMP_TOTEM_MAX		3
#define TEMP_OTHER_START	4

/* Max entries in DEVMNG header */
#define MAX_ENTRY_NUM		32

struct devmng_entry {
	u32 type;
	u32 offset;
	u32 len;
} __packed;

struct ascend_hwmon_data {
	void __iomem *shm_base;		/* maps LPM_SHM_PHYS */
	struct platform_device *pdev;
	u32 temp_offset;			/* byte offset of temp data within shm */
	u32 temp_len;				/* byte length of temp region */
};

/*
 * Parse the DEVMNG header to find the temperature entry offset.
 * Returns 0 on success.
 */
static int ascend_hwmon_parse_header(struct ascend_hwmon_data *data)
{
	void __iomem *devmng = data->shm_base + DEVMNG_OFFSET;
	u32 magic, entry_num;
	struct devmng_entry entry;

	magic = readl(devmng);
	if (magic != LPM_SHAREMEM_MAGIC) {
		pr_err("ascend_hwmon: bad DEVMNG magic: 0x%08x (expected 0x%08x)\n",
		       magic, LPM_SHAREMEM_MAGIC);
		return -EINVAL;
	}

	/* skip version (offset 4), read entry_num (offset 8) */
	entry_num = readl(devmng + 8);
	if (entry_num == 0 || entry_num > MAX_ENTRY_NUM) {
		pr_err("ascend_hwmon: invalid entry_num: %u\n", entry_num);
		return -EINVAL;
	}

	if (ENTRY_TYPE_TEMP >= entry_num) {
		pr_err("ascend_hwmon: no temperature entry (entry_num=%u)\n", entry_num);
		return -ENOENT;
	}

	/* entries start at offset 12, each is 12 bytes */
	entry.type   = readl(devmng + 12 + ENTRY_TYPE_TEMP * 12);
	entry.offset = readl(devmng + 12 + ENTRY_TYPE_TEMP * 12 + 4);
	entry.len    = readl(devmng + 12 + ENTRY_TYPE_TEMP * 12 + 8);

	if (entry.type != ENTRY_TYPE_TEMP) {
		pr_err("ascend_hwmon: entry[0].type=%u, expected %u\n",
		       entry.type, ENTRY_TYPE_TEMP);
		return -EINVAL;
	}

	/* temp data is at DEVMNG_OFFSET + entry.offset within shm */
	data->temp_offset = (u32)DEVMNG_OFFSET + entry.offset;
	data->temp_len = entry.len;

	pr_info("ascend_hwmon: temp entry offset=0x%x len=%u (%u sensors)\n",
		data->temp_offset, data->temp_len,
		data->temp_len / (u32)sizeof(u16));
	return 0;
}

static int ascend_hwmon_read_temp(struct ascend_hwmon_data *data,
				  int channel, long *val)
{
	u32 byte_off = (u32)channel * sizeof(u16);
	u16 raw;

	if (byte_off + sizeof(u16) > data->temp_len)
		return -EOPNOTSUPP;

	raw = readw(data->shm_base + data->temp_offset + byte_off);

	/* 0xFFFF means sensor not ready */
	if (raw == 0xFFFF)
		return -EAGAIN;

	/* hwmon expects millidegrees Celsius */
	*val = (long)raw * 1000;
	return 0;
}

static int ascend_hwmon_read(struct device *dev, enum hwmon_sensor_types type,
			     u32 attr, int channel, long *val)
{
	struct ascend_hwmon_data *data = dev_get_drvdata(dev);

	if (type != hwmon_temp || attr != hwmon_temp_input)
		return -EOPNOTSUPP;

	return ascend_hwmon_read_temp(data, channel, val);
}

static umode_t ascend_hwmon_is_visible(const void *data,
				       enum hwmon_sensor_types type,
				       u32 attr, int channel)
{
	if (type != hwmon_temp || channel >= 4)
		return 0;
	if (attr == hwmon_temp_input || attr == hwmon_temp_label)
		return 0444;
	return 0;
}

static const char * const ascend_hwmon_temp_labels[] = {
	"SoC Temperature",	/* TEMP_SOC_MAX — btop picks this up as CPU temp */
	"AI Core Max",		/* TEMP_AIC_MAX */
	"HBM Max",		/* TEMP_HBM_MAX */
	"Totem Max",		/* TEMP_TOTEM_MAX */
};

static int ascend_hwmon_read_string(struct device *dev,
				    enum hwmon_sensor_types type,
				    u32 attr, int channel,
				    const char **str)
{
	if (type != hwmon_temp || attr != hwmon_temp_label || channel >= 4)
		return -EOPNOTSUPP;

	*str = ascend_hwmon_temp_labels[channel];
	return 0;
}

static const struct hwmon_channel_info * const ascend_hwmon_info[] = {
	HWMON_CHANNEL_INFO(temp,
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
	int ret;

	g_hwmon_data = kzalloc(sizeof(*g_hwmon_data), GFP_KERNEL);
	if (!g_hwmon_data)
		return -ENOMEM;

	g_hwmon_data->shm_base = ioremap(LPM_SHM_PHYS, LPM_SHM_SIZE);
	if (!g_hwmon_data->shm_base) {
		pr_err("ascend_hwmon: failed to ioremap LPM shared memory\n");
		kfree(g_hwmon_data);
		return -ENOMEM;
	}

	ret = ascend_hwmon_parse_header(g_hwmon_data);
	if (ret) {
		pr_err("ascend_hwmon: failed to parse DEVMNG header (%d)\n", ret);
		iounmap(g_hwmon_data->shm_base);
		kfree(g_hwmon_data);
		return ret;
	}

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
	pr_info("ascend_hwmon: registered (LPM shm @ 0x%llx, temp @ +0x%x)\n",
		(unsigned long long)LPM_SHM_PHYS, g_hwmon_data->temp_offset);
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
