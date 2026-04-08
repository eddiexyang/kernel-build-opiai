SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c
.ONESHELL:
.SILENT:

ROOT_DIR ?= $(CURDIR)
OUTPUT_DIR ?= $(ROOT_DIR)/output
WORKSPACE_DIR ?= $(ROOT_DIR)/workspace

ARCH_TYPE ?= arm64
CROSS_COMPILE_PREFIX ?= aarch64-linux-gnu-
JOBS ?= $(shell nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 16)
CCACHE_BIN ?= $(shell command -v ccache 2>/dev/null || true)
CCACHE ?= $(CCACHE_BIN)

KERNEL_CC := $(if $(strip $(CCACHE_BIN)),$(strip $(CCACHE_BIN)) $(CROSS_COMPILE_PREFIX)gcc,$(CROSS_COMPILE_PREFIX)gcc)
KERNEL_HOSTCC := $(if $(strip $(CCACHE_BIN)),$(strip $(CCACHE_BIN)) gcc,gcc)
KERNEL_HOSTCXX := $(if $(strip $(CCACHE_BIN)),$(strip $(CCACHE_BIN)) g++,g++)

LINUX_SOURCE_DIR ?= $(ROOT_DIR)/linux-source
KERNEL_TOOLS_DIR := $(ROOT_DIR)/kernel-tools
KERNEL_SPMI_DIR := $(KERNEL_TOOLS_DIR)/spmi_hisi
DRIVER_SOURCE_DIR := $(ROOT_DIR)/driver
DTB_SOURCE_DIR := $(ROOT_DIR)/dtb
ABL_DIR := $(ROOT_DIR)/abl
LIBC_SEC_DIR := $(ROOT_DIR)/libc_sec
SIGNING_DIR := $(ROOT_DIR)/signing
CONFIG_FEATURE_FILE := $(ROOT_DIR)/config/feature/ascend310Brc.config

KERNEL_WORKSPACE := $(LINUX_SOURCE_DIR)
DTB_WORKSPACE := $(WORKSPACE_DIR)/dtb

KERNEL_DEFCONFIG := ascend310B_defconfig
DRIVER_PRODUCT := ascend310Brc
DTB_PRODUCT := ascend310Brc
HEADER_VERSION := 0.0.0.0.0
KERNEL_RELEASE_SUFFIX ?= opiai
KERNEL_LOCALVERSION := $(if $(strip $(KERNEL_RELEASE_SUFFIX)),-$(strip $(KERNEL_RELEASE_SUFFIX)),)
KERNEL_MAKE_VARS = ARCH="$(ARCH_TYPE)" CROSS_COMPILE="$(CROSS_COMPILE_PREFIX)" LOCALVERSION="$(KERNEL_LOCALVERSION)" CC="$(KERNEL_CC)" HOSTCC="$(KERNEL_HOSTCC)" HOSTCXX="$(KERNEL_HOSTCXX)"
BUILD_TIMESTAMP_MINUTE ?= $(shell date '+%Y%m%d%H%M')
MODULES_DEB_PACKAGE_ARCH ?= $(if $(filter arm64,$(ARCH_TYPE)),arm64,$(if $(filter amd64 x86_64,$(ARCH_TYPE)),amd64,$(ARCH_TYPE)))

.DEFAULT_GOAL := help

include $(ROOT_DIR)/build/common.mk
include $(ROOT_DIR)/build/kernel.mk
include $(ROOT_DIR)/build/dtb.mk
include $(ROOT_DIR)/build/driver.mk
include $(ROOT_DIR)/build/modules_deb.mk
include $(ROOT_DIR)/build/modules_apk.mk
include $(ROOT_DIR)/build/headers_deb.mk
include $(ROOT_DIR)/build/headers_apk.mk

.PHONY: help clean
help:
	printf '%s\n' '[Usage]:'
	printf '%-50s%-50s\n' '    make clean' '# build clean'
	printf '%-50s%-50s\n' '    make kernel' '# build kernel for Euler'
	printf '%-50s%-50s\n' '    make dtb' '# build dtb'
	printf '%-50s%-50s\n' '    make driver' '# build driver'
	printf '%-50s%-50s\n' '    make modules-deb' '# build merged kernel/driver modules deb'
	printf '%-50s%-50s\n' '    make modules-apk' '# build merged kernel/driver modules apk (OpenWrt)'
	printf '%-50s%-50s\n' '    make headers-deb' '# build linux-headers deb for external modules'
	printf '%-50s%-50s\n' '    make headers-apk' '# build linux-headers apk for OpenWrt'
	printf '%-50s%-50s\n' "    default linux source" "# $(LINUX_SOURCE_DIR)"

clean:
	$(MAKE) --no-print-directory clean-all-outputs
