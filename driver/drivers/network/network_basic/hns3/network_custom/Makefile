# SPDX-License-Identifier: GPL-2.0+
#
# Makefile for the HISILICON network device drivers.
#

# Add security options
EXTRA_CFLAGS += -DMIRROR_MEMORY_SUPPORT

KERNEL_DIR := $(srctree)
ccflags-y += -fstack-protector-all
ccflags-y += -Wl,-z,relro,-z,now
ccflags-y += -Wl,-z,noexecstack
ccflags-y += -D_FORTIFY_SOURCE=2 -O2
ccflags-y += -fvisibility=hidden
ccflags-y += -Wformat=2 -Wfloat-equal
ccflags-y += -fsigned-char

HNS3_DIR = $(KERNEL_DIR)/drivers/net/ethernet/hisilicon/hns3
ccflags-y += -I$(HNS3_DIR) \
		-I$(HNS3_DIR)/hns3pf \
		-I$(HNS3_DIR)/hns3_common \
		-I$(TOP_DIR)/abl/bbox/inc/bbox/device \
		-I$(TOP_DIR)/libc_sec/include \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/inc \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/fpdc \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/config \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/sensor \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/sensor/config \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/event \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/dfm_template \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/include \
		-I$(TOP_DIR)/inc/driver

obj-m += network.o
network-objs =	network_custom.o \
			hns3_bbox.o \
			hclge_profiling.o \
			hclge_i2c.o \
			hclge_serdes_common.o \
			hclge_serdes_adapt.o \
			hclge_serdes_rate_switch.o \
			hclge_fault_manager.o \
			hclge_serdes_loopback.o \
			hclge_tm.o \
			hns3_mirror_mem.o
