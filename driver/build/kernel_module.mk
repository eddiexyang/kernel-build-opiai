
my_ko_module_name := $(LOCAL_MODULE)
my_ko_module_target := $(KO_TYPE).$(my_ko_module_name)

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


define ko_extra_symvers
$(strip $(foreach m,$(1),$(PWD)/$($(KO_TYPE)_OUT_INTERMEDIATES)/$(strip $(m))_ko/Module.symvers))
endef

define normalize-local-extra-symvers
$(strip $(if $(strip $(1)),$(abspath $(1))))
endef

ifneq ($(strip $(symbol_check)), false)
LOCAL_DEPEND_VALID_KO := $(addprefix $(KO_TYPE).,$(LOCAL_DEPEND_KO))
LOCAL_DEPEND_SYMVERS := $(call ko_extra_symvers,$(LOCAL_DEPEND_KO))
LOCAL_DEPEND_KERNEL := $(HOST_KERNEL_VERSION_SYMVERS)
endif

$(KO_MODULES_OUT_INTER)/Module.symvers: $(KO_KERNEL_TIMESTAMP)
	@:

$(KO_KERNEL_TIMESTAMP): PRIVATE_KERNEL_SYMBOLS_PATH := $(LOCAL_DEPEND_KERNEL)
$(KO_KERNEL_TIMESTAMP): PRIVATE_DEPEND_KO := $(strip $(LOCAL_DEPEND_KO))
$(KO_KERNEL_TIMESTAMP): PRIVATE_LOCAL_EXTRA_SYMBOLS := $(call normalize-local-extra-symvers,$(LOCAL_KBUILD_EXTRA_SYMBOLS))
$(KO_KERNEL_TIMESTAMP): PRIVATE_EXTRA_SYMBOLS = $(strip $(call ko_extra_symvers,$(PRIVATE_DEPEND_KO)) $(PRIVATE_LOCAL_EXTRA_SYMBOLS))
$(KO_KERNEL_TIMESTAMP): PRIVATE_PRODUCT_SIDE := $(PRODUCT_SIDE)
$(KO_KERNEL_TIMESTAMP): PRIVATE_KO_TYPE := $(KO_TYPE)
$(KO_KERNEL_TIMESTAMP): PRIVATE_KO_NAME := $(my_ko_module_name)
$(KO_KERNEL_TIMESTAMP): PRIVATE_KERNEL_MODULES_OUT := $(KO_MODULES_OUT_INTER)
$(KO_KERNEL_TIMESTAMP): PRIVATE_KERNEL_DIR := $(KO_DEP_KERNEL_DIR)
$(KO_KERNEL_TIMESTAMP): PRIVATE_COMPILER_PREFIX := $(CROSS_COMPILER_PREFIX)
$(KO_KERNEL_TIMESTAMP): PRIVATE_MODULE_DIR := $(MODULE_DIR)
$(KO_KERNEL_TIMESTAMP): PRIVATE_BUILD_KO_MODULES := $(BUILT_KO_MODULES)
$(KO_KERNEL_TIMESTAMP): PRIVATE_ARCH_TYPE := $(KO_ARCH_TYPE)
$(KO_KERNEL_TIMESTAMP): PRIVATE_COMMON_CPPFLAGS := $(strip $(COMMON_KO_CPPFLAGS))
$(KO_KERNEL_TIMESTAMP): $(KERNEL_KO_DOT_CONFIG) $(LOCAL_DEPEND_VALID_KO) $(LOCAL_DEPEND_SYMVERS) $(LOCAL_DEPEND_KERNEL)
	@mkdir -p $(PRIVATE_KERNEL_MODULES_OUT)
	@if [ -n "$(strip $(PRIVATE_KERNEL_SYMBOLS_PATH))" ]; then \
		cp -f $(PRIVATE_KERNEL_SYMBOLS_PATH) $(PRIVATE_KERNEL_MODULES_OUT)/; \
		cp -f $(PRIVATE_KERNEL_SYMBOLS_PATH) $(PRIVATE_KERNEL_DIR)/Module.symvers; \
	fi

ifneq ($(KERNEL_KO_CONFIG_FILE),)
	@$(MAKE) -C $(PRIVATE_KERNEL_DIR) ARCH=$(PRIVATE_ARCH_TYPE) CROSS_COMPILE=$(PRIVATE_COMPILER_PREFIX) modules_prepare
	@$(MAKE) $(INC_N) -C $(PRIVATE_KERNEL_DIR) M=$(PRIVATE_MODULE_DIR) CROSS_COMPILE=$(PRIVATE_COMPILER_PREFIX) ARCH=$(PRIVATE_ARCH_TYPE) KBUILD_EXTRA_SYMBOLS="$(PRIVATE_EXTRA_SYMBOLS)" KBUILD_EXTRA_CPPFLAGS="$(PRIVATE_COMMON_CPPFLAGS)" FEATURE_MK_PATH=$(FEATURE_MK_PATH) PRODUCT_SIDE=$(PRIVATE_PRODUCT_SIDE) OPIAI_KO_TYPE=$(PRIVATE_KO_TYPE) modules
else
	@$(MAKE) $(INC_N) -C $(PRIVATE_KERNEL_DIR) M=$(PRIVATE_MODULE_DIR) O='' CROSS_COMPILE=$(PRIVATE_COMPILER_PREFIX) ARCH=$(PRIVATE_ARCH_TYPE) KBUILD_EXTRA_SYMBOLS="$(PRIVATE_EXTRA_SYMBOLS)" KBUILD_EXTRA_CPPFLAGS="$(PRIVATE_COMMON_CPPFLAGS)" FEATURE_MK_PATH=$(FEATURE_MK_PATH) PRODUCT_SIDE=$(PRIVATE_PRODUCT_SIDE) OPIAI_KO_TYPE=$(PRIVATE_KO_TYPE) modules
endif   
	@$(PRIVATE_COMPILER_PREFIX)strip -S --remove-section=.note.gnu.build-id $(PRIVATE_BUILD_KO_MODULES)
	@cp -f $(PRIVATE_BUILD_KO_MODULES) $(PRIVATE_KERNEL_MODULES_OUT)/$(notdir $(PRIVATE_BUILD_KO_MODULES))
		@if [ -f "$(PRIVATE_MODULE_DIR)/Module.symvers" ]; then \
			cp -f $(PRIVATE_MODULE_DIR)/Module.symvers $(PRIVATE_KERNEL_MODULES_OUT)/Module.symvers; \
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
