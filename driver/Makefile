# SPDX-License-Identifier: GPL-2.0
#
# Ascend310B Driver - Flattened Build System with Parallel Compilation
#
# Usage:
#   make driver_device KERNEL_DIR=... CROSS_COMPILE=... PRODUCT=... build_device=true [-jN]
#   make driver_host   KERNEL_DIR=... CROSS_COMPILE=... PRODUCT=... [-jN]
#   make clean

SHELL := /bin/bash

DRIVER_DIR := $(CURDIR)
KERNEL_DIR ?= $(DRIVER_DIR)/kernel/linux-source
CROSS_COMPILE ?= aarch64-linux-gnu-
ARCH ?= arm64
PRODUCT ?= ascend310Brc
PRODUCT_SIDE ?= device

-include $(DRIVER_DIR)/config/feature_config/feature.mk

OUT_DEVICE := $(DRIVER_DIR)/out/device
OUT_HOST := $(DRIVER_DIR)/out/host

# Common kernel module build command
KMAKE = $(MAKE) -C $(KERNEL_DIR) \
	ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) \
	PRODUCT=$(PRODUCT) PRODUCT_SIDE=$(PRODUCT_SIDE) \
	FEATURE_MK_PATH=$(DRIVER_DIR)/config/feature_config/feature.mk \
	KBUILD_MODPOST_WARN=1

# ============================================================================
# Per-module build targets (enables parallel make -jN)
# ============================================================================

# Generate a build target for each module directory
define module_target
.PHONY: build-$(1)
build-$(1):
	@if [ -f "$(DRIVER_DIR)/$(1)/Kbuild" ]; then \
		echo "  [BUILD] $(1)"; \
		$(KMAKE) PRODUCT_SIDE=$(PRODUCT_SIDE) M=$(DRIVER_DIR)/$(1) modules \
			|| echo "  [FAIL] $(1)"; \
	else \
		echo "  [SKIP] $(1) (no Kbuild)"; \
	fi
endef

# ============================================================================
# Device-side modules (dependency layers for ordering)
# ============================================================================

DEVICE_L0 := bbox dbl_board_config dbl_cfg_persistent dbl_chip_config \
	dbl_algorithm dbl_dev_identity dbl_runenv_config \
	soc_resmng uda urd event_sched xsmem \
	davinci_intf icm ascend_ctl pkicms fpdc memory soc_misc drv_notify \
	soft_fault ipcdrv pcie_rc_sr sysmonitor svm \
	trs_id_allocator trs_id_pool trs_tsmng trs_mia_agent \
	i2c_uniform sys_ctrl_uniform usb_uniform spi_uniform gpio_uniform pwm \
	mttcan eeprom serdes \
	dms_mng dms_dtm dms_smf hwmon

DEVICE_L1 := trs_chan trs_stars trs_sub_stars dvpp_osal dfm

DEVICE_L2 := hns3 ts_platform ts_platform_host upgrade user_cfg hiusbc pcie ts_channel \
	audio vdp mipi_rx

DEVICE_L3 := dms virtmng virtmng_resmgr virtmng_host_stub vpcie vpc_agent trs_core

DEVICE_L4 := hdcdrv hdcdrv_host tsdrv tsdrv_host vtsdrv dp_proc_mng dp_proc_mng_host trs_cdqm trs_pm_adapt trs_kunpeng

ifneq ($(filter $(PRODUCT), ascend910 ascend310p),)
DEVICE_L4 += vhdc
endif

DEVICE_L5 := logdrv prof trs_adapt_agent trs_mia_adapt

DEVICE_EXTRA := lpm/drv_lpm lpm/pm lpm/drv_lpm_devmng trs_shrid trs_sec_eh soc_platform
DEVICE_EXTRA += kbox

ALL_DEVICE := $(DEVICE_L0) $(DEVICE_L1) $(DEVICE_L2) $(DEVICE_L3) \
	$(DEVICE_L4) $(DEVICE_L5) $(DEVICE_EXTRA)

# Generate targets for all modules
$(foreach mod,$(ALL_DEVICE),$(eval $(call module_target,$(mod))))

# Modules that import symbols from other out-of-tree modules must wait for the
# provider's Module.symvers to be generated before running modpost.
build-dfm: build-bbox
build-ts_channel: build-dfm
build-pcie: build-dbl_runenv_config build-dfm
build-vpc_agent: build-vpcie
build-vhdc: build-vpcie build-vpc_agent
build-vtsdrv: build-vpcie build-vpc_agent

# Layer targets: each layer depends on the previous, but modules within a layer are parallel
.PHONY: device-l0 device-l1 device-l2 device-l3 device-l4 device-l5 device-extra

device-l0: $(addprefix build-,$(DEVICE_L0))
device-l1: device-l0 $(addprefix build-,$(DEVICE_L1))
device-l2: device-l1 $(addprefix build-,$(DEVICE_L2))
device-l3: device-l2 $(addprefix build-,$(DEVICE_L3))
device-l4: device-l3 $(addprefix build-,$(DEVICE_L4))
device-l5: device-l4 $(addprefix build-,$(DEVICE_L5))
device-extra: device-l5 $(addprefix build-,$(DEVICE_EXTRA))

# ============================================================================
# Main targets
# ============================================================================

.PHONY: driver_device driver_host clean

driver_device: PRODUCT_SIDE := device
driver_device:
	@mkdir -p $(OUT_DEVICE)
	@echo "=== Building device-side modules (parallel within layers) ==="
	$(MAKE) device-extra PRODUCT_SIDE=device
	@echo "=== Collecting device .ko files ==="
	@find $(DRIVER_DIR) -name '*.ko' -not -path '*/linux-source/*' -not -path '*/out/*' \
		-exec cp {} $(OUT_DEVICE)/ \; 2>/dev/null
	@echo "Device modules: $$(ls $(OUT_DEVICE)/*.ko 2>/dev/null | wc -l) .ko files"

driver_host: PRODUCT_SIDE := host
driver_host:
	@mkdir -p $(OUT_HOST)
	@echo "=== Building host-side modules ==="
	@echo "Host build not yet implemented (modules share same Kbuild)"
	@echo "Host modules: 0 .ko files"

clean:
	@rm -rf $(OUT_DEVICE) $(OUT_HOST) out/
	@find $(DRIVER_DIR) \( -name '*.o' -o -name '*.ko' -o -name '*.mod' \
		-o -name '*.mod.c' -o -name '.*.cmd' -o -name 'Module.symvers' \
		-o -name 'modules.order' -o -name '.tmp_versions' \
		-o -name '.cache.mk' -o -name '.module-common.o' \) \
		-not -path '*/linux-source/*' -exec rm -rf {} + 2>/dev/null || true
	@echo "Driver tree cleaned"
