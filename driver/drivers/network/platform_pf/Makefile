# SPDX-License-Identifier: GPL-2.0+
#
# Makefile for the HISILICON network device drivers.
#

# Add security options
ifeq ($(TOP_DIR),)
KERNEL_DIR := $(srctree)
TOP_DIR := $(PWD)/
endif

KERNEL_DIR := $(srctree)
ccflags-y += -fstack-protector-strong
ccflags-y += -Wl,-z,relro,-z,now
ccflags-y += -Wl,-z,noexecstack
ccflags-y += -D_FORTIFY_SOURCE=2 -O2
ccflags-y += -fvisibility=hidden
ccflags-y += -Wformat=2 -Wfloat-equal
ccflags-y += -fsigned-char
ccflags-y += -DHCLGE_PLATFORM_DEV

ifeq ($(PRODUCT),ascend310Besl)
ccflags-y += -DCONFIG_PLATFORM_ESL
endif

ifeq ($(PRODUCT),ascend310Bemu)
ccflags-y += -DCONFIG_PLATFORM_EMU
endif

ifeq ($(PRODUCT),ascend310B)
ccflags-y += -DCONFIG_PLATFORM_ASIC
ccflags-y += -DCONFIG_PLATFORM_SUPPORT_AUTONEG
ccflags-y += -DCONFIG_PLATFORM_SUPPORT_MAC_SGMII_AUTONEG
ccflags-y += -DPLATFORM_SUPPORT_EEPROM_MAC
endif

ifeq ($(PRODUCT),ascend310Brc)
ccflags-y += -DCONFIG_PLATFORM_ASIC
ccflags-y += -DCONFIG_PLATFORM_SUPPORT_AUTONEG
ccflags-y += -DCONFIG_PLATFORM_SUPPORT_MAC_SGMII_AUTONEG
ccflags-y += -DPLATFORM_SUPPORT_EEPROM_MAC
endif

ifeq ($(PRODUCT),as31xm1)
ccflags-y += -DCONFIG_PLATFORM_MDC
ccflags-y += -DCONFIG_PLATFORM_ASIC
ccflags-y += -DCONFIG_PLATFORM_SUPPORT_MAC_SGMII_AUTONEG
ccflags-y += -DPLATFORM_SUPPORT_EEPROM_MAC
endif

CONFIG_HCLGE_PLF := m
HNS3_DIR = $(KERNEL_DIR)/drivers/net/ethernet/hisilicon/hns3
PWD = $(TOP_DIR)/drivers/network/platform_pf
#add include path
ccflags-y += 	-I$(HNS3_DIR) \
		-I$(HNS3_DIR)/hns3_common \
		-I$(HNS3_DIR)/hns3pf \
		-I$(PWD) \
		-I$(PWD)/include/mac \
		-I$(PWD)/include/reg \
		-I$(PWD)/gpio \
		-I$(PWD)/mdio \
		-I$(TOP_DIR)/drivers/ai_sdk/driver/linux/kernel_space/src/serdes_drv/serdes_h25/includes \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/fpdc \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/config \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/inc \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/sensor \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/sensor/config \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/event \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/dfm_template \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/include \
		-I$(TOP_DIR)/inc/driver \
		-I$(TOP_DIR)/abl/bbox/inc/bbox/device \
		-I$(TOP_DIR)/libc_sec/include \
		-I$(TOP_DIR)/drivers/ai_sdk/driver/linux/kernel_space/src/eeprom

#### compile hclgeplf.ko
obj-$(CONFIG_HCLGE_PLF) += hclgeplf.o
hclgeplf-objs := platform_mac.o
hclgeplf-objs += mdio/hns_mdio_common.o
hclgeplf-objs += mdio/hns_mdio_dc.o
hclgeplf-objs += platform_phy.o
hclgeplf-objs += hclge_plf_flow_director.o
hclgeplf-objs += hclge_plf_main.o
hclgeplf-objs += hclge_dcb.o
hclgeplf-objs += hclge_plf_tm.o
hclgeplf-objs += hclge_plf_rss.o
hclgeplf-objs += gpio/gpio.o
hclgeplf-objs += hclge_plf_debugfs.o
hclgeplf-objs += hclge_plf_profiling.o
hclgeplf-objs += hclge_plf_err.o
hclgeplf-objs += hclge_plf_bbox.o
hclgeplf-objs += hclge_plf_qos.o
