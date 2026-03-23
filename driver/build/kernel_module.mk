
my_ko_module_name := $(LOCAL_MODULE)
my_ko_module_target := $(KO_TYPE).$(my_ko_module_name)
my_ko_type := $(KO_TYPE)
my_local_depend_ko := $(strip $(LOCAL_DEPEND_KO))

my_installed_ko_modules := $(LOCAL_INSTALLED_KO_FILES)

#check if define module repeatly
ko_id := MODULE_KO.$(KO_TYPE).$(my_ko_module_name)
ifdef $(ko_id)
  $(error $(my_ko_module_name) has alredy defined)
endif
$(ko_id) := $(my_ko_module_name)

BUILT_KO_MODULES := $(addprefix $(LOCAL_KO_SRC_FOLDER)/, $(notdir $(my_installed_ko_modules)))
INSTALLD_KO_MOUDLES := $(addprefix $($(KO_TYPE)_OUT_ROOT)/, $(my_installed_ko_modules))
KO_DEP_KERNEL_DIR := $($(KO_TYPE)_KERNEL_PATH)
KO_MODULES_OUT_INTER := $(PWD)/$($(KO_TYPE)_OUT_INTERMEDIATES)/$(strip $(my_ko_module_name))_ko
KO_KERNEL_TIMESTAMP := $(KO_MODULES_OUT_INTER)/.timestamp
KERNEL_KO_DOT_CONFIG := $(KO_MODULES_OUT_INTER)/.config

ifneq ($(strip $($(KO_TYPE)_KERNEL_COMPILER_PREFIX)),)
CROSS_COMPILER_PREFIX := $($(KO_TYPE)_KERNEL_COMPILER_PREFIX)
else
CROSS_COMPILER_PREFIX :=
endif

MODULE_DIR := $(addprefix $(PWD)/,$(LOCAL_KO_SRC_FOLDER))
KERNEL_KO_CONFIG_FILE := $($(KO_TYPE)_KERNEL_DEFCONFIG)

KERNEL_ARCH_CONFIGS_DIR := $(dir $(KERNEL_KO_CONFIG_FILE))

KO_GEN_CONFIG_PATH := $(KERNEL_ARCH_CONFIGS_DIR)/hw_$(TARGET_PRODUCT)_ko_defconfig

ifneq ($(KERNEL_KO_CONFIG_FILE),)
# Use the shared kernel source .config so that modules_prepare (run once via
# KO_KERNEL_PREPARE_STAMP below) writes into the same tree that module builds
# read from.  Keeping this as the kernel-dir .config matches the original
# in-place design and avoids "source tree not clean" errors from the kernel
# Kbuild when O= is pointed at a pre-built in-place tree.
KERNEL_KO_DOT_CONFIG := $(KO_DEP_KERNEL_DIR)/.config
endif

export TOP_DIR := $(abspath $(PWD))
export DRIVER_USER_DIR := $(abspath $(TOP_DIR)/abl/ascend_hal/user_space)
export DRIVER_OPEN_USER_DIR := $(abspath $(TOP_DIR)/abl/ascend_hal/user_space_open)
export DRIVER_CHIP_KERNEL_DIR := $(abspath $(TOP_DIR)/drivers/ai_sdk/driver/linux/kernel_space)
export DRIVER_KERNEL_UNIFORM_DIR := $(abspath $(TOP_DIR)/drivers/ai_sdk/driver/linux/kernel_space_uniform)
export DRIVER_KERNEL_DIR := $(abspath $(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space)
export DRIVER_OPEN_KERNEL_DIR := $(abspath $(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space_open)

COMMON_KO_CPPFLAGS := \
	-include $(abspath $(PWD)/kernel/linux-source/include/linux/opiai_vendor_compat.h) \
	-I$(TOP_DIR)/inc \
	-I$(TOP_DIR)/inc/driver \
	-I$(TOP_DIR)/libc_sec/include \
	-I$(TOP_DIR)/drivers/network/platform_pf \
	-I$(TOP_DIR)/drivers/network/platform_pf/include/mac \
	-I$(TOP_DIR)/drivers/network/platform_pf/include/reg \
	-I$(TOP_DIR)/drivers/network/platform_pf/mdio \
	-I$(DRIVER_KERNEL_DIR)/inc \
	-I$(DRIVER_KERNEL_DIR)/inc/dbl \
	-I$(DRIVER_KERNEL_DIR)/src/common \
	-I$(DRIVER_KERNEL_DIR)/src/fpdc \
	-I$(DRIVER_KERNEL_DIR)/src/rms/trs_drv/lba/comm/adapt

ifeq ($(KO_TYPE),HOST)
COMMON_KO_CPPFLAGS += -DOPIAI_DRIVER_HOST_BUILD
else ifeq ($(KO_TYPE),DEVICE)
COMMON_KO_CPPFLAGS += -DOPIAI_DRIVER_DEVICE_BUILD
endif

ifneq ($(KERNEL_KO_CONFIG_FILE),)
$(KO_GEN_CONFIG_PATH): $(KERNEL_KO_CONFIG_FILE)
	cp -f $<  $@

$(KERNEL_KO_DOT_CONFIG): $(KO_GEN_CONFIG_PATH)
	@+make -C $(KO_DEP_KERNEL_DIR) ARCH=$(KO_ARCH_TYPE) CROSS_COMPILE=$(CROSS_COMPILER_PREFIX) $(notdir $<)
	@+make -C $(KO_DEP_KERNEL_DIR) ARCH=$(KO_ARCH_TYPE) CROSS_COMPILE=$(CROSS_COMPILER_PREFIX) olddefconfig
else
$(KERNEL_KO_DOT_CONFIG):
ifneq ($(KO_DEP_KERNEL_DIR),)
	@mkdir -p $(dir $@)
	cp -f $(KO_DEP_KERNEL_DIR)/.config $@
else
	@echo "Error: please specify the KERNEL_DIR!" && exit 1
endif
endif

HOST_KERNEL_VERSION_SYMVERS := $(HOST_OUT_INTERMEDIATES)/symvers/$($(KO_TYPE)_KERNEL_PATH)/Module.symvers

$(HOST_KERNEL_VERSION_SYMVERS):
	@mkdir -p $(dir $@)
	@if [ -f "${KERNEL_DIR}/Module.symvers" ]; then \
		cp -f "${KERNEL_DIR}/Module.symvers" "$@"; \
	elif [ -f "${KERNEL_DIR}/vmlinux.symvers" ]; then \
		cp -f "${KERNEL_DIR}/vmlinux.symvers" "$@"; \
	else \
		echo "Error: missing kernel symvers under ${KERNEL_DIR}" >&2; \
		exit 1; \
	fi

# ---------------------------------------------------------------------------
# Shared kernel-prepare stamp (one per KO_TYPE)
#
# modules_prepare and the Module.symvers copy both write into the shared
# kernel source tree.  Running them once (serialised by this stamp) and then
# letting individual module compilations proceed in parallel is safe because
# "make M=<dir> modules" only writes inside <dir>.
# ---------------------------------------------------------------------------
KO_KERNEL_PREPARE_STAMP := $(PWD)/$($(KO_TYPE)_OUT_INTERMEDIATES)/kernel_prepare.stamp

ifndef KERNEL_PREPARE_STAMP_$(KO_TYPE)_DEFINED
KERNEL_PREPARE_STAMP_$(KO_TYPE)_DEFINED := 1

$(KO_KERNEL_PREPARE_STAMP): PRIVATE_KERNEL_DIR     := $(KO_DEP_KERNEL_DIR)
$(KO_KERNEL_PREPARE_STAMP): PRIVATE_ARCH_TYPE       := $(KO_ARCH_TYPE)
$(KO_KERNEL_PREPARE_STAMP): PRIVATE_COMPILER_PREFIX := $(CROSS_COMPILER_PREFIX)
$(KO_KERNEL_PREPARE_STAMP): PRIVATE_KERNEL_CC       := $(if $(strip $(CCACHE)),$(strip $(CCACHE)) $(CROSS_COMPILER_PREFIX)gcc,$(CROSS_COMPILER_PREFIX)gcc)
$(KO_KERNEL_PREPARE_STAMP): PRIVATE_HOSTCC          := $(if $(strip $(CCACHE)),$(strip $(CCACHE)) gcc,gcc)
$(KO_KERNEL_PREPARE_STAMP): PRIVATE_HOSTCXX         := $(if $(strip $(CCACHE)),$(strip $(CCACHE)) g++,g++)
$(KO_KERNEL_PREPARE_STAMP): PRIVATE_SYMVERS_SRC     := $(HOST_KERNEL_VERSION_SYMVERS)
$(KO_KERNEL_PREPARE_STAMP): $(KERNEL_KO_DOT_CONFIG) $(HOST_KERNEL_VERSION_SYMVERS)
	@mkdir -p $(dir $@)
	@if [ -n "$(strip $(PRIVATE_SYMVERS_SRC))" ]; then \
		cp -f $(PRIVATE_SYMVERS_SRC) $(PRIVATE_KERNEL_DIR)/Module.symvers; \
	fi
ifneq ($(KERNEL_KO_CONFIG_FILE),)
	@$(MAKE) -C $(PRIVATE_KERNEL_DIR) ARCH=$(PRIVATE_ARCH_TYPE) CROSS_COMPILE=$(PRIVATE_COMPILER_PREFIX) CC="$(PRIVATE_KERNEL_CC)" HOSTCC="$(PRIVATE_HOSTCC)" HOSTCXX="$(PRIVATE_HOSTCXX)" modules_prepare
endif
	@touch $@

endif # KERNEL_PREPARE_STAMP_$(KO_TYPE)_DEFINED

define ko_extra_symvers
$(strip $(foreach m,$(1),$(PWD)/$($(2)_OUT_INTERMEDIATES)/$(strip $(m))_ko/Module.symvers))
endef

define normalize-local-extra-symvers
$(strip $(if $(strip $(1)),$(abspath $(1))))
endef

define normalize-all-extra-symvers
$(strip $(sort $(foreach s,$(1),$(if $(strip $(s)),$(abspath $(s))))))
endef

ifneq ($(strip $(symbol_check)), false)
LOCAL_DEPEND_VALID_KO := $(addprefix $(my_ko_type).,$(my_local_depend_ko))
LOCAL_DEPEND_SYMVERS := $(call ko_extra_symvers,$(my_local_depend_ko),$(my_ko_type))
LOCAL_DEPEND_KERNEL := $(HOST_KERNEL_VERSION_SYMVERS)
endif

$(KO_MODULES_OUT_INTER)/Module.symvers: $(KO_KERNEL_TIMESTAMP)
	@:

# Extra symvers paths declared via LOCAL_KBUILD_EXTRA_SYMBOLS need to be Make
# prerequisites so that parallel builds wait for those modules to finish before
# this module's MODPOST step tries to open them.
#
# Only track symvers that live in the CURRENT type's intermediates directory.
# Cross-type symvers (e.g., a HOST module referencing a DEVICE symvers file)
# are handled by build ordering (device build always precedes host build in
# driver.mk) and must NOT be added as Make dependencies here, because the
# cross-type phony targets (e.g., DEVICE.foo) are undefined in the other
# type's build context and would cause "no rule to make target" errors.
MY_KO_INTERMEDIATES_DIR := $(PWD)/$($(KO_TYPE)_OUT_INTERMEDIATES)
LOCAL_KBUILD_EXTRA_SYMBOLS_DEPS := $(filter $(MY_KO_INTERMEDIATES_DIR)/%,$(call normalize-local-extra-symvers,$(LOCAL_KBUILD_EXTRA_SYMBOLS)))

$(KO_KERNEL_TIMESTAMP): PRIVATE_KERNEL_SYMBOLS_PATH := $(LOCAL_DEPEND_KERNEL)
$(KO_KERNEL_TIMESTAMP): PRIVATE_DEPEND_KO := $(my_local_depend_ko)
# Full list for KBUILD_EXTRA_SYMBOLS (cross-type symvers must still be passed
# to the compiler); filtered list used only as Make prerequisites above.
$(KO_KERNEL_TIMESTAMP): PRIVATE_LOCAL_EXTRA_SYMBOLS := $(call normalize-local-extra-symvers,$(LOCAL_KBUILD_EXTRA_SYMBOLS))
$(KO_KERNEL_TIMESTAMP): PRIVATE_EXTRA_SYMBOLS = $(call normalize-all-extra-symvers,$(call ko_extra_symvers,$(PRIVATE_DEPEND_KO),$(PRIVATE_KO_TYPE)) $(PRIVATE_LOCAL_EXTRA_SYMBOLS))
$(KO_KERNEL_TIMESTAMP): PRIVATE_PRODUCT_SIDE := $(PRODUCT_SIDE)
$(KO_KERNEL_TIMESTAMP): PRIVATE_KO_TYPE := $(KO_TYPE)
$(KO_KERNEL_TIMESTAMP): PRIVATE_KO_NAME := $(my_ko_module_name)
$(KO_KERNEL_TIMESTAMP): PRIVATE_KERNEL_MODULES_OUT := $(KO_MODULES_OUT_INTER)
$(KO_KERNEL_TIMESTAMP): PRIVATE_KERNEL_DIR := $(KO_DEP_KERNEL_DIR)
$(KO_KERNEL_TIMESTAMP): PRIVATE_COMPILER_PREFIX := $(CROSS_COMPILER_PREFIX)
$(KO_KERNEL_TIMESTAMP): PRIVATE_CCACHE := $(strip $(CCACHE))
$(KO_KERNEL_TIMESTAMP): PRIVATE_KERNEL_CC := $(if $(strip $(CCACHE)),$(strip $(CCACHE)) $(CROSS_COMPILER_PREFIX)gcc,$(CROSS_COMPILER_PREFIX)gcc)
$(KO_KERNEL_TIMESTAMP): PRIVATE_HOSTCC := $(if $(strip $(CCACHE)),$(strip $(CCACHE)) gcc,gcc)
$(KO_KERNEL_TIMESTAMP): PRIVATE_HOSTCXX := $(if $(strip $(CCACHE)),$(strip $(CCACHE)) g++,g++)
$(KO_KERNEL_TIMESTAMP): PRIVATE_MODULE_DIR := $(MODULE_DIR)
$(KO_KERNEL_TIMESTAMP): PRIVATE_BUILD_KO_MODULES := $(BUILT_KO_MODULES)
$(KO_KERNEL_TIMESTAMP): PRIVATE_ARCH_TYPE := $(KO_ARCH_TYPE)
$(KO_KERNEL_TIMESTAMP): PRIVATE_COMMON_CPPFLAGS := $(strip $(COMMON_KO_CPPFLAGS))

$(KO_KERNEL_TIMESTAMP): $(KO_KERNEL_PREPARE_STAMP) $(LOCAL_DEPEND_VALID_KO) $(LOCAL_DEPEND_SYMVERS) $(LOCAL_KBUILD_EXTRA_SYMBOLS_DEPS)
	@mkdir -p $(PRIVATE_KERNEL_MODULES_OUT)
	@rm -f $(PRIVATE_KERNEL_MODULES_OUT)/Module.symvers

ifneq ($(KERNEL_KO_CONFIG_FILE),)
	@$(MAKE) $(INC_N) -C $(PRIVATE_KERNEL_DIR) M=$(PRIVATE_MODULE_DIR) CROSS_COMPILE=$(PRIVATE_COMPILER_PREFIX) CC="$(PRIVATE_KERNEL_CC)" HOSTCC="$(PRIVATE_HOSTCC)" HOSTCXX="$(PRIVATE_HOSTCXX)" ARCH=$(PRIVATE_ARCH_TYPE) KBUILD_EXTRA_SYMBOLS="$(PRIVATE_EXTRA_SYMBOLS)" KBUILD_EXTRA_CPPFLAGS="$(PRIVATE_COMMON_CPPFLAGS)" FEATURE_MK_PATH=$(FEATURE_MK_PATH) PRODUCT_SIDE=$(PRIVATE_PRODUCT_SIDE) OPIAI_KO_TYPE=$(PRIVATE_KO_TYPE) modules
else
	@$(MAKE) $(INC_N) -C $(PRIVATE_KERNEL_DIR) M=$(PRIVATE_MODULE_DIR) O='' CROSS_COMPILE=$(PRIVATE_COMPILER_PREFIX) CC="$(PRIVATE_KERNEL_CC)" HOSTCC="$(PRIVATE_HOSTCC)" HOSTCXX="$(PRIVATE_HOSTCXX)" ARCH=$(PRIVATE_ARCH_TYPE) KBUILD_EXTRA_SYMBOLS="$(PRIVATE_EXTRA_SYMBOLS)" KBUILD_EXTRA_CPPFLAGS="$(PRIVATE_COMMON_CPPFLAGS)" FEATURE_MK_PATH=$(FEATURE_MK_PATH) PRODUCT_SIDE=$(PRIVATE_PRODUCT_SIDE) OPIAI_KO_TYPE=$(PRIVATE_KO_TYPE) modules
endif
	@$(PRIVATE_COMPILER_PREFIX)strip -S --remove-section=.note.gnu.build-id $(PRIVATE_BUILD_KO_MODULES)
	@cp -f $(PRIVATE_BUILD_KO_MODULES) $(PRIVATE_KERNEL_MODULES_OUT)/$(notdir $(PRIVATE_BUILD_KO_MODULES))
		@if [ -f "$(PRIVATE_MODULE_DIR)/Module.symvers" ]; then \
			awk '$$3 != "vmlinux"' "$(PRIVATE_MODULE_DIR)/Module.symvers" > "$(PRIVATE_KERNEL_MODULES_OUT)/Module.symvers"; \
		else \
			: > $(PRIVATE_KERNEL_MODULES_OUT)/Module.symvers; \
		fi
	rm -rf $(PRIVATE_MODULE_DIR)/.*.cmd $(PRIVATE_MODULE_DIR)/*.o $(PRIVATE_MODULE_DIR)/.tmp_versions $(PRIVATE_MODULE_DIR)/Module.symvers $(PRIVATE_MODULE_DIR)/modules.order $(PRIVATE_MODULE_DIR)/$(PRIVATE_KO_NAME).ko $(PRIVATE_MODULE_DIR)/$(PRIVATE_KO_NAME).mod.c $(PRIVATE_MODULE_DIR)/.cache.mk
	@touch $@

$(INSTALLD_KO_MOUDLES): $(KO_KERNEL_TIMESTAMP)
	@mkdir -p $(dir $@)
	@cp -f $(dir $<)/$(notdir $@) $@

.PHONY: $(my_ko_module_target)
$(my_ko_module_target): $(INSTALLD_KO_MOUDLES)


ifeq ($(strip $(LOCAL_IS_TEST_MODULE)),true)
ALL_$(KO_TYPE)_TEST_KO_FILES += $(INSTALLD_KO_MOUDLES)
else
ALL_$(KO_TYPE)_KO_FILES += $(INSTALLD_KO_MOUDLES)
endif
