/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* Description:
* Author: huawei
* Create: 2022-7-15
*/

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>
#include <linux/uaccess.h>

#include "securec.h"

#include "uda_notifier.h"
#include "uda_dev.h"

#define PROC_FS_MODE 0400

static inline const char *uda_get_chip_name(u32 type)
{
    static const char *chip_name[HISI_CHIP_NUM] = {
        [HISI_MINI_V1] = "mini_v1",
        [HISI_CLOUD_V1] = "cloud_v1",
        [HISI_MINI_V2] = "mini_v2",
        [HISI_CLOUD_V2] = "cloud_v2",
        [HISI_MINI_V3] = "mini_v3",
    };

    return (type < HISI_CHIP_NUM) ? chip_name[type] : "unknown";
}

static void uda_udev_single_show(struct seq_file *seq, struct uda_dev_inst *dev_inst)
{
    char buf[TYPE_BUF_LEN];

    uda_type_to_string(&dev_inst->type, buf, TYPE_BUF_LEN);

    seq_printf(seq, "udevid %u type(%s) status '%s, %s' added_udevid %u chip_type %s\n",
        dev_inst->udevid, buf, uda_is_suspend_status(dev_inst->status) ? "suspend" : "ready",
        uda_is_mia_status(dev_inst->status) ? "mia" : "sia",
        dev_inst->para.udevid, uda_get_chip_name(dev_inst->para.chip_type));

    if (dev_inst->para.remote_udevid != UDA_INVALID_UDEVID) {
        seq_printf(seq, "        remote udevid %u\n", dev_inst->para.remote_udevid);
    }

    if (dev_inst->para.dev != NULL) {
        seq_printf(seq, "        has os device\n");
    }

    if (uda_is_dev_shared(dev_inst->udevid)) {
        struct uda_access_share_node *share_node = NULL;
        seq_printf(seq, "        shared dev used by ns id:");
        mutex_lock(&dev_inst->access.mutex);
        list_for_each_entry(share_node, &dev_inst->access.share_head, node) {
            seq_printf(seq, " %u", share_node->ns_id);
        }
        seq_printf(seq, "\n");
        mutex_unlock(&dev_inst->access.mutex);
    } else {
        if (dev_inst->access.ns != NULL) {
            seq_printf(seq, "        has used by container, ns id %u\n", dev_inst->access.ns_id);
        }
    }

    if (!uda_is_phy_dev(dev_inst->udevid)) {
        seq_printf(seq, "        mia para: phy_devid %u sub_devid %u\n",
            dev_inst->mia_para.phy_devid, dev_inst->mia_para.sub_devid);
    }

    if (dev_inst->agent_dev != NULL) {
        seq_printf(seq, "        has agent device\n");
    }

    seq_printf(seq, "\n");
}

static int uda_udev_show(struct seq_file *seq, void *offset)
{
    u32 i;

    for (i = 0; i < UDA_UDEV_MAX_NUM; i++) {
        struct uda_dev_inst *dev_inst = uda_dev_inst_get(i);
        if (dev_inst != NULL) {
            uda_udev_single_show(seq, dev_inst);
            uda_dev_inst_put(dev_inst);
        }
    }

    return 0;
}

int uda_udev_open(struct inode *inode, struct file *file)
{
    return single_open(file, uda_udev_show, PDE_DATA(inode));
}

void uda_ns_node_single_show(struct uda_ns_node *ns_node, void *priv)
{
    struct seq_file *seq = (struct seq_file *)priv;
    u32 devid;

    seq_printf(seq, "ns_id %u identify %llx root_tgid %u dev_num %u namespace %pK, udev list:\n",
        ns_node->ns_id, ns_node->identify, ns_node->root_tgid, ns_node->dev_num, ns_node->ns);
    seq_printf(seq, "logic_devid udevid\n");
    for (devid = 0; devid < ns_node->dev_num; devid++) {
        seq_printf(seq, "%8u %8u\n", devid, ns_node->devid_to_udevid[devid]);
    }
    seq_printf(seq, "\n\n");
}

static int uda_ns_node_show(struct seq_file *seq, void *offset)
{
    uda_for_each_ns_node_safe(seq, uda_ns_node_single_show);
    return 0;
}

int uda_ns_node_open(struct inode *inode, struct file *file)
{
    return single_open(file, uda_ns_node_show, PDE_DATA(inode));
}

static void _uda_notifier_show(struct uda_dev_type *type, struct uda_notifiers *notifiers, void *priv)
{
    struct seq_file *seq = (struct seq_file *)priv;
    char buf[TYPE_BUF_LEN];
    int i;
    bool has_notifier = false;

    down_read(&notifiers->sem);
    for (i = 0; i < UDA_PRI_MAX; i++) {
        if (!list_empty(&notifiers->pri_head[i])) {
            has_notifier = true;
            break;
        }
    }

    if (has_notifier) {
        uda_type_to_string(type, buf, TYPE_BUF_LEN);
        seq_printf(seq, "notifier type: %s\n", buf);
    }

    for (i = 0; i < UDA_PRI_MAX; i++) {
        struct list_head *nf_head = &notifiers->pri_head[i];
        struct uda_notifier_node *nf = NULL;

        if (!list_empty(nf_head)) {
            seq_printf(seq, "    pri %d\n", i);

            list_for_each_entry(nf, nf_head, node) {
                seq_printf(seq, "        notifier: %s\n", nf->notifier);
            }
        }
    }
    up_read(&notifiers->sem);
    if (has_notifier) {
        seq_printf(seq, "\n");
    }
}

static int uda_notifier_show(struct seq_file *seq, void *offset)
{
    uda_for_each_notifiers(seq, _uda_notifier_show);
    return 0;
}

int uda_notifier_open(struct inode *inode, struct file *file)
{
    return single_open(file, uda_notifier_show, PDE_DATA(inode));
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0)
static const struct file_operations udevice_ops = {
    .owner = THIS_MODULE,
    .open    = uda_udev_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static const struct file_operations namespace_node_ops = {
    .owner = THIS_MODULE,
    .open    = uda_ns_node_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static const struct file_operations notifier_ops = {
    .owner = THIS_MODULE,
    .open    = uda_notifier_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

#else

static const struct proc_ops udevice_ops = {
    .proc_open    = uda_udev_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static const struct proc_ops namespace_node_ops = {
    .proc_open    = uda_ns_node_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static const struct proc_ops notifier_ops = {
    .proc_open    = uda_notifier_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};
#endif

void uda_proc_fs_init(void)
{
    struct proc_dir_entry *top_entry = proc_mkdir("uda", NULL);
    if (top_entry != NULL) {
        (void)proc_create_data("udevice", PROC_FS_MODE, top_entry, &udevice_ops, NULL);
        (void)proc_create_data("namespace_node", PROC_FS_MODE, top_entry, &namespace_node_ops, NULL);
        (void)proc_create_data("notifiers", PROC_FS_MODE, top_entry, &notifier_ops, NULL);
    }
}

void uda_proc_fs_uninit(void)
{
    (void)remove_proc_subtree("uda", NULL);
}

