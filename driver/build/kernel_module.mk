
my_ko_module_name := $(LOCAL_MODULE)

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

export TOP_DIR=${KERNEL_DIR}/../../
export DRIVER_USER_DIR=${KERNEL_DIR}/../../abl/ascend_hal/user_space
export DRIVER_OPEN_USER_DIR=${KERNEL_DIR}/../../abl/ascend_hal/user_space_open
export DRIVER_CHIP_KERNEL_DIR=${KERNEL_DIR}/../../drivers/ai_sdk/driver/linux/kernel_space
export DRIVER_KERNEL_UNIFORM_DIR=${KERNEL_DIR}/../../drivers/ai_sdk/driver/linux/kernel_space_uniform
export DRIVER_KERNEL_DIR=${KERNEL_DIR}/../../drivers/ai_sdk/arc/linux/kernel_space
export DRIVER_OPEN_KERNEL_DIR=${KERNEL_DIR}/../../drivers/ai_sdk/arc/linux/kernel_space_open

ifneq ($(KERNEL_KO_CONFIG_FILE),)
$(KO_GEN_CONFIG_PATH): $(KERNEL_KO_CONFIG_FILE)
	cp -f $<  $@

$(KERNEL_KO_DOT_CONFIG): $(KO_GEN_CONFIG_PATH)
	@mkdir -p $(dir $@)
	@+make -C $(PRIVATE_KERNEL_DIR) O=$(PRIVATE_KERNEL_MODULES_OUT) ARCH=$(PRIVATE_ARCH_TYPE) CROSS_COMPILE=$(PRIVATE_COMPILER_PREFIX) $(notdir $<)
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
	@cp -rf ${KERNEL_DIR}/Module.symvers $@


KBUILD_EXTRA_SYMBOLS_VAL :=
$(foreach m,$(LOCAL_DEPEND_KO),$(eval KBUILD_EXTRA_SYMBOLS_VAL += $(addprefix $(PWD)/$($(KO_TYPE)_OUT_INTERMEDIATES)/, $(strip $(m))_ko/Module.symvers)))

ifneq ($(strip $(symbol_check)), false)
ifneq ($(KERNEL_KO_CONFIG_FILE),)
LOCAL_DEPEND_VALID_KO :=
LOCAL_DEPEND_KERNEL :=
else
LOCAL_DEPEND_VALID_KO := $(LOCAL_DEPEND_KO)
LOCAL_DEPEND_KERNEL := $($(KO_TYPE)_OUT_INTERMEDIATES)/symvers/$($(KO_TYPE)_KERNEL_PATH)/Module.symvers
endif
endif

$(KO_KERNEL_TIMESTAMP): PRIVATE_KERNEL_SYMBOLS_PATH := $(LOCAL_DEPEND_KERNEL)
$(KO_KERNEL_TIMESTAMP): PRIVATE_EXTRA_SYMBOLS := $(strip $(KBUILD_EXTRA_SYMBOLS_VAL))
$(KO_KERNEL_TIMESTAMP): PRIVATE_KO_NAME := $(my_ko_module_name)
$(KO_KERNEL_TIMESTAMP): PRIVATE_KERNEL_MODULES_OUT := $(KO_MODULES_OUT_INTER)
$(KO_KERNEL_TIMESTAMP): PRIVATE_KERNEL_DIR := $(KO_DEP_KERNEL_DIR)
$(KO_KERNEL_TIMESTAMP): PRIVATE_COMPILER_PREFIX := $(CROSS_COMPILER_PREFIX)
$(KO_KERNEL_TIMESTAMP): PRIVATE_MODULE_DIR := $(MODULE_DIR)
$(KO_KERNEL_TIMESTAMP): PRIVATE_BUILD_KO_MODULES := $(BUILT_KO_MODULES)
$(KO_KERNEL_TIMESTAMP): PRIVATE_ARCH_TYPE := $(KO_ARCH_TYPE)
$(KO_KERNEL_TIMESTAMP): $(KERNEL_KO_DOT_CONFIG) $(LOCAL_DEPEND_VALID_KO) $(LOCAL_DEPEND_KERNEL)
	@mkdir -p $(dir $@)
	@-cp -rf $(PRIVATE_KERNEL_SYMBOLS_PATH) $(PRIVATE_KERNEL_MODULES_OUT)

ifneq ($(KERNEL_KO_CONFIG_FILE),)
	@$(MAKE) -C $(PRIVATE_KERNEL_DIR) O=$(PRIVATE_KERNEL_MODULES_OUT) ARCH=$(PRIVATE_ARCH_TYPE) CROSS_COMPILE=$(PRIVATE_COMPILER_PREFIX) modules_prepare
	@$(MAKE) $(INC_N) -C $(PRIVATE_KERNEL_DIR) M=$(PRIVATE_MODULE_DIR) O=$(PRIVATE_KERNEL_MODULES_OUT) CROSS_COMPILE=$(PRIVATE_COMPILER_PREFIX) ARCH=$(PRIVATE_ARCH_TYPE) KBUILD_EXTRA_SYMBOLS="$(PRIVATE_EXTRA_SYMBOLS)" FEATURE_MK_PATH=$(FEATURE_MK_PATH) PRODUCT_SIDE=$(PRODUCT_SIDE) modules
else
	@$(MAKE) $(INC_N) -C $(PRIVATE_KERNEL_DIR) M=$(PRIVATE_MODULE_DIR) O='' CROSS_COMPILE=$(PRIVATE_COMPILER_PREFIX) ARCH=$(PRIVATE_ARCH_TYPE) KBUILD_EXTRA_SYMBOLS="$(PRIVATE_EXTRA_SYMBOLS)" FEATURE_MK_PATH=$(FEATURE_MK_PATH) PRODUCT_SIDE=$(PRODUCT_SIDE) modules
endif   
	@$(PRIVATE_COMPILER_PREFIX)strip -S --remove-section=.note.gnu.build-id $(PRIVATE_BUILD_KO_MODULES)
	@cp -rf $(PRIVATE_BUILD_KO_MODULES) $(dir $@)
	@cp -rf $(PRIVATE_MODULE_DIR)/Module.symvers $(dir $@)
	rm -rf $(PRIVATE_MODULE_DIR)/.*.cmd $(PRIVATE_MODULE_DIR)/*.o $(PRIVATE_MODULE_DIR)/.tmp_versions $(PRIVATE_MODULE_DIR)/Module.symvers $(PRIVATE_MODULE_DIR)/modules.order $(PRIVATE_MODULE_DIR)/$(PRIVATE_KO_NAME).ko $(PRIVATE_MODULE_DIR)/$(PRIVATE_KO_NAME).mod.c $(PRIVATE_MODULE_DIR)/.cache.mk
	@touch $@

$(INSTALLD_KO_MOUDLES): $(KO_KERNEL_TIMESTAMP)
	@mkdir -p $(dir $@)
	@cp -rf $(dir $<)/$(notdir $@) $@

.PHONY:$(my_ko_module_name) 
$(my_ko_module_name): $(INSTALLD_KO_MOUDLES)


ifeq ($(strip $(LOCAL_IS_TEST_MODULE)),true)
ALL_$(KO_TYPE)_TEST_KO_FILES += $(INSTALLD_KO_MOUDLES)
else
ALL_$(KO_TYPE)_KO_FILES += $(INSTALLD_KO_MOUDLES)
endif
