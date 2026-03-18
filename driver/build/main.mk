
.PHONY: all
DEFAULT_GOAL := all
$(DEFAULT_GOAL):

BUILD_SYSTEM := build
TOPDIR :=

PWD := $(shell pwd)

include $(BUILD_SYSTEM)/help.mk

include $(BUILD_SYSTEM)/config.mk

include $(BUILD_SYSTEM)/definitions.mk

-include $(BUILD_SYSTEM)/product_modules/host.mk

-include $(BUILD_SYSTEM)/product_modules/host-arm64.mk

-include $(BUILD_SYSTEM)/product_modules/device-ctrlcpu.mk

include $(BUILD_SYSTEM)/release.mk

ifeq ($(filter help clean,$(MAKECMDGOALS)),)
#search all modules in system and include 
subdir_module_makefiles := $(shell find . -name module.mk | sort)

$(foreach mk, $(subdir_module_makefiles), $(eval include $(mk)))
endif

all: release_modules
	@echo "\n\033[32m### build successfully ###\033[0m"

.PHONY: toolchain
toolchain: toolchain_log toolchain_ide_daemon toolchain_debug upgrade_tool toolchain_profiling toolchain_pctrace
	@echo "\n\033[32m### build successfully ###\033[0m"

.PHONY: hiaiengine
hiaiengine: mmpa third_party
	@echo "\n\033[32m### build successfully ###\033[0m"

.PHONY: driver
driver: driver_host
	@echo "\n\033[32m### build successfully ###\033[0m"

.PHONY: clean
clean:
	@rm -rf out
	@rm -rf kernel/linux-source/arch/arm64/configs/hw_cloud_ko_defconfig
	@rm -rf kernel/linux-source/arch/arm64/configs/hw_mini_ko_defconfig

