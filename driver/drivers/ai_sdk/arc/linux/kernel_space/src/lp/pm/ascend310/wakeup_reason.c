/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
*/
#include <linux/securec.h>
#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/suspend.h>
#include "wakeup_reason.h"

static DEFINE_SPINLOCK(resume_reason_lock);

static int g_irq_count;

/* monotonic time before last suspend */
static ktime_t g_last_monotime;
/* monotonic time after last suspend */
static ktime_t g_curr_monotime;
/* monotonic boottime offset before last suspend */
static ktime_t g_last_stime;
/* monotonic boottime offset after last suspend */
static ktime_t g_curr_stime;

#ifdef CONFIG_HISI_SR_DEBUG_SLEEP
static int g_irq_list[MAX_WAKEUP_REASON_IRQS];
static struct kobject *g_wakeup_reason = NULL;

/* get the resume irq information */
static ssize_t last_resume_reason_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	int irq_no = 0;
	int buf_offset = 0;
	char irq_name[IRQ_NAME_LEN] = {0};
	int ret = 0;

	if (buf == NULL)
		return -ENOMEM;

	spin_lock(&resume_reason_lock);
	for (irq_no = 0; irq_no < g_irq_count; irq_no++) {
		if (get_irq_name(g_irq_list[irq_no])) {
			ret = strncpy_s(irq_name, (IRQ_NAME_LEN - 1), get_irq_name(g_irq_list[irq_no]),
				(IRQ_NAME_LEN - 2));
			if (ret != EOK) {
				pr_err("string copy has error, irq is %d, total number is %d\n",
					irq_no, g_irq_count);
				return buf_offset;
			}

			ret = sprintf_s(buf + buf_offset, IRQ_NAME_LEN, "%d %s\n",
							g_irq_list[irq_no], irq_name);
			if (ret < EOK) {
				pr_err("sprintf_s has eror, irq is %d, total number is %d\n",
					irq_no, g_irq_count);
				return -EINVAL;
			}

			buf_offset += ret;
		} else {
			ret = sprintf_s(buf + buf_offset, IRQ_NUM_LEN, "%d\n", g_irq_list[irq_no]);
			if (ret < EOK) {
				pr_err("sprintf_s has eror, irq is %d, total number is %d\n",
					irq_no, g_irq_count);
				return -EINVAL;
			}

			buf_offset += ret;
		}
	}
	spin_unlock(&resume_reason_lock);

	return buf_offset;
}

/* get the SR time information */
static ssize_t last_suspend_time_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	struct timespec64 sleep_time;
	struct timespec64 total_time;
	struct timespec64 suspend_resume_time;
	ssize_t ret = 0;

	if (buf == NULL)
		return -ENOMEM;

	/*
	 * total_time is calculated from monotonic bootoffsets because
	 * unlike CLOCK_MONOTONIC it include the time spent in suspend state.
	 */
	total_time = ktime_to_timespec64(ktime_sub(g_curr_stime, g_last_stime));

	/*
	 * suspend_resume_time is calculated as monotonic (CLOCK_MONOTONIC)
	 * time interval before entering suspend and post suspend.
	 */
	suspend_resume_time = ktime_to_timespec64(ktime_sub(g_curr_monotime, g_last_monotime));

	/* sleep_time can get from total_time minus suspend_resume_time */
	sleep_time = timespec64_sub(total_time, suspend_resume_time);

	/* Export suspend_resume_time and sleep_time in pair here. */
	ret = sprintf_s(buf, WAKEUP_DEBUG_LEN, "%lu.%09lu %lu.%09lu\n",
					suspend_resume_time.tv_sec, suspend_resume_time.tv_nsec,
					sleep_time.tv_sec, sleep_time.tv_nsec);
	if (ret < EOK)
		pr_err("suspend time show sprintf_s has error\n");

	return ret;
}

static struct kobj_attribute resume_reason = __ATTR_RO(last_resume_reason);
static struct kobj_attribute suspend_time = __ATTR_RO(last_suspend_time);

static struct attribute *attrs[] = {
	&resume_reason.attr,
	&suspend_time.attr,
	NULL,
};
static struct attribute_group attr_group = {
	.attrs = attrs,
};

/*
 * logs all the wake up reasons to the kernel
 * stores the irqs to expose them to the userspace via sysfs
 */
void log_wakeup_reason(int irq)
{
	struct irq_desc *desc = NULL;

	desc = irq_to_desc(irq);
	if ((desc != NULL) && (desc->action != NULL) &&
		(desc->action->name != NULL))
		pr_info("Resume caused by IRQ %d, %s\n", irq,
			desc->action->name);
	else
		pr_info("Resume caused by IRQ %d\n", irq);

	spin_lock(&resume_reason_lock);
	if (g_irq_count == MAX_WAKEUP_REASON_IRQS) {
		spin_unlock(&resume_reason_lock);
		pr_warn("Resume caused by more than %d IRQs\n",
				MAX_WAKEUP_REASON_IRQS);
		return;
	}

	g_irq_list[g_irq_count++] = irq;
	spin_unlock(&resume_reason_lock);
}
#endif

/* Detects a suspend and clears all the previous wake up reasons*/
static int wakeup_reason_pm_event(struct notifier_block *notifier,
			unsigned long pm_event, void *unused)
{
	switch (pm_event) {
	case PM_SUSPEND_PREPARE:
		spin_lock(&resume_reason_lock);
		g_irq_count = 0;
		spin_unlock(&resume_reason_lock);
		/* monotonic time since boot */
		g_last_monotime = ktime_get();
		/*
		* monotonic time since boot including
		* the time spent in suspend
		*/
		g_last_stime = ktime_get_boottime();
		break;
	case PM_POST_SUSPEND:
		/* monotonic time since boot */
		g_curr_monotime = ktime_get();
		/*
		* monotonic time since boot including
		* the time spent in suspend
		*/
		g_curr_stime = ktime_get_boottime();
		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block wakeup_reason_pm_notifier_block = {
	.notifier_call = wakeup_reason_pm_event,
};

/*
 * Initializes the sysfs parameter
 * registers the pm_event notifier
 */
int wakeup_reason_init(void)
{
	int retval = 0;

	/* regiter PM notifer which will know the pm status */
	retval = register_pm_notifier(&wakeup_reason_pm_notifier_block);
	if (retval)
		pr_warn("[%s] failed to register PM notifier %d\n", __func__, retval);

#ifdef CONFIG_HISI_SR_DEBUG_SLEEP
	/* create a file node to check SR information */
	g_wakeup_reason = kobject_create_and_add("wakeup_reasons", kernel_kobj);
	if (g_wakeup_reason == NULL) {
		pr_err("[%s] failed to create a sysfs kobject\n",
				__func__);
		return -ENOMEM;
	}

	retval = sysfs_create_group(g_wakeup_reason, &attr_group);
	if (retval) {
		kobject_put(g_wakeup_reason);
		pr_warn("[%s] failed to create a sysfs group %d\n",
				__func__, retval);
	}
#endif

	return RTN_SUCCESS;
}
