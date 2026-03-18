
ifeq ($(strip $(LOCAL_IS_DEVICE_MODULE)),true)
  my_kind := DEVICE
else
  my_kind := HOST
endif

my_module_name := $(strip $(LOCAL_MODULE))

intermediates := $(call intermediates-dir-for,$($(my_kind)_OUT_INTERMEDIATES),$(LOCAL_MODULE_CLASS),$(LOCAL_MODULE))

LOCAL_BUILT_MODULE := $(intermediates)/$(LOCAL_MODULE)$(LOCAL_MODULE_SUFFIX)

intermediates_lib_dir := $($(my_kind)_OUT_INTERMEDIATES)/lib

OVERRIDE_BUILT_MODULE_PATH := $(intermediates_lib_dir)

ifeq ($(strip $(LOCAL_MODULE_CLASS)),SHARED_LIBRARIES)
  ifneq ($(strip $(LOCAL_SONAME_SUFFIX)),)
    # if has configured LOCAL_SONAME_SUFFIX, 
    # then set build target under $(OVERRIDE_BUILT_MODULE_PATH) to
    # LOCAL_BUILT_MODULE, without $(LOCAL_SONAME_SUFFIX) suffix
    LOCAL_BUILT_MODULE := $(OVERRIDE_BUILT_MODULE_PATH)/$(LOCAL_MODULE)$(LOCAL_MODULE_SUFFIX)
  else
    # otherwise copy $(LOCAL_BUILT_MODULE) to
    # file under $(OVERRIDE_BUILT_MODULE_PATH)
$(eval $(call copy-one-file,$(LOCAL_BUILT_MODULE),$(OVERRIDE_BUILT_MODULE_PATH)/$(LOCAL_MODULE)$(LOCAL_MODULE_SUFFIX)))
  endif
endif


ifneq (true,$(LOCAL_UNINSTALLABLE_MODULE))
  LOCAL_INSTALLED_MODULE := $($(my_kind)_OUT_ROOT)/$(LOCAL_MODULE)$(LOCAL_MODULE_SUFFIX)
  ifneq ($(LOCAL_INSTALLED_PATH),)
    LOCAL_INSTALLED_MODULE := $(LOCAL_INSTALLED_PATH)
  endif

  ifneq ($(LOCAL_SONAME_SUFFIX),)
    LOCAL_INSTALLED_SO_LINK := $(LOCAL_INSTALLED_MODULE)
    LOCAL_INSTALLED_MODULE := $(addprefix $(dir $(LOCAL_INSTALLED_MODULE)), $(LOCAL_MODULE)$(LOCAL_SONAME_SUFFIX))

$(LOCAL_INSTALLED_SO_LINK): $(LOCAL_INSTALLED_MODULE)
	@echo "Soft-Link: $@ -> $<"
	@ln -sf $(<F) $@

  endif

$(my_module_name): $(LOCAL_INSTALLED_MODULE)
endif

ifeq ($(LOCAL_CC),gcc)
  $(LOCAL_INSTALLED_MODULE): PRIVATE_STRIP := strip
else
  $(LOCAL_INSTALLED_MODULE): PRIVATE_STRIP := $($(my_prefix)STRIP)
endif

$(LOCAL_INSTALLED_MODULE): PRIVATE_INSTALLED_MODULE_UNSTRIPED := $(LOCAL_INSTALLED_MODULE_UNSTRIPED)
$(LOCAL_INSTALLED_MODULE): $(LOCAL_BUILT_MODULE)
	@echo "Install: $@"
ifeq ($(strip $(LOCAL_MODULE_CLASS)),FOLDER)
	$(copy-files-to-folder)
else
	$(copy-file-to-target)
ifeq ($(strip),y)
ifeq ($(filter $(my_module_name),$(NOSTRIP_MODULE)),)
ifneq ($(filter EXECUTABLES SHARED_LIBRARIES,$(LOCAL_MODULE_CLASS)),)
	$(transform-to-stripped)
	$(call copy-imgs-to-spc-out,debug_imgs,$@.debug)
endif #endif of filter
endif #endif of NOSTRIP_MODULE_FILTER
endif #endif of strip
endif #endif of folder

#check if define module repeatly
module_id := MODULE.$(if $(LOCAL_IS_DEVICE_MODULE),DEVICE,HOST).$(my_module_name).$(LOCAL_MODULE_CLASS)
ifdef $(module_id)
  $(error $(my_module_name) has alredy defined)
endif
$(module_id) := $(LOCAL_BUILT_MODULE)

.PHONY: $(my_module_name)
$(my_module_name): $(LOCAL_BUILT_MODULE)
	@echo -e "\n\n\033[32m### build $@ successfully ###\033[0m"

LOCAL_MODULE_DEPENDENCY := \
  $(addprefix $(OVERRIDE_BUILT_MODULE_PATH)/, \
    $(addsuffix $(TARGET_SHLIB_SUFFIX), \
      $(LOCAL_SHARED_LIBRARIES)))


ALL_MODULES += $(my_module_name)
ifneq ($(LOCAL_MODULE_CLASS),STATIC_LIBRARIES)
  ALL_MODULES.$(LOCAL_INSTALLED_MODULE).PATH := $(ALL_MODULES.$(my_module_name).PATH) $(OVERRIDE_BUILT_MODULE_PATH)/$(LOCAL_MODULE)$(LOCAL_MODULE_SUFFIX)
  ALL_MODULES.$(LOCAL_INSTALLED_MODULE).CLASS := $(LOCAL_MODULE_CLASS)
  ALL_MODULES.$(LOCAL_INSTALLED_MODULE).DEPENDENCY := $(LOCAL_MODULE_DEPENDENCY)
else
  ALL_MODULES.$(LOCAL_BUILT_MODULE).PATH := $(LOCAL_BUILT_MODULE)
  ALL_MODULES.$(LOCAL_BUILT_MODULE).CLASS := $(LOCAL_MODULE_CLASS)
endif

ALL_MODULES.$(my_module_name).BUILT := $(ALL_MODULES.$(my_module_name).BUILT) $(LOCAL_BUILT_MODULE)
ifneq ($(strip $(LOCAL_SONAME_SUFFIX)),)
  ALL_MODULES.$(my_module_name).BUILT_SONAME := $(addprefix $(dir $(LOCAL_BUILT_MODULE)),$(addsuffix $(LOCAL_SONAME_SUFFIX),$(LOCAL_MODULE)))
endif

ifneq ($(LOCAL_INSTALLED_MODULE),)
  ALL_MODULES.$(my_module_name).INSTALLED := $(ALL_MODULES.$(my_module_name).INSTALLED) $(LOCAL_INSTALLED_MODULE)
endif
