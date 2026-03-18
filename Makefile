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

LINUX_SOURCE_DIR ?= $(abspath $(ROOT_DIR)/../linux-opiai)
PATCH_ROOT_DIR := $(ROOT_DIR)/patches
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
KERNEL_RELEASE_SUFFIX ?= HwHiAiUser
KERNEL_LOCALVERSION := $(if $(strip $(KERNEL_RELEASE_SUFFIX)),-$(strip $(KERNEL_RELEASE_SUFFIX)),)
MODULES_DEB_PACKAGE_ARCH ?= $(if $(filter arm64,$(ARCH_TYPE)),arm64,$(if $(filter amd64 x86_64,$(ARCH_TYPE)),amd64,$(ARCH_TYPE)))

.DEFAULT_GOAL := help

include $(ROOT_DIR)/build/common.mk
include $(ROOT_DIR)/build/kernel.mk
include $(ROOT_DIR)/build/dtb.mk
include $(ROOT_DIR)/build/driver.mk
include $(ROOT_DIR)/build/modules_deb.mk
include $(ROOT_DIR)/build/headers_deb.mk

.PHONY: help clean
help:
	printf '%s\n' '[Usage]:'
	printf '%-50s%-50s\n' '    make clean' '# build clean'
	printf '%-50s%-50s\n' '    make kernel-patch' '# sync in-place kernel source inputs'
	printf '%-50s%-50s\n' '    make kernel' '# build kernel for Euler'
	printf '%-50s%-50s\n' '    make dtb' '# build dtb'
	printf '%-50s%-50s\n' '    make driver' '# build driver'
	printf '%-50s%-50s\n' '    make modules-deb' '# build merged kernel/driver modules deb'
	printf '%-50s%-50s\n' '    make headers-deb' '# build linux-headers deb for external modules'
	printf '%-50s%-50s\n' "    default linux source" "# $(LINUX_SOURCE_DIR)"

clean:
	$(MAKE) --no-print-directory clean-all-outputs
