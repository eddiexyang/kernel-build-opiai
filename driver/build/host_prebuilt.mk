ifeq ($(strip $(LOCAL_MODULE_SUFFIX)),)
LOCAL_MODULE_SUFFIX := $(suffix $(LOCAL_SRC_FILES))
endif

ifeq ($(filter $(LOCAL_MODULE_CLASS),EXECUTABLES SHARED_LIBRARIES STATIC_LIBRARIES FOLDER ETC),)
  $(error you should specify LOCAL_MODULE_CLASS which value must be EXECUTABLES, SHARED_LIBRARIES, STATIC_LIBRARIES FOLDER or ETC)
endif

ifneq ($(LLT_TEST),)
include $(BUILD_SYSTEM)/llt_base_rules.mk
else
include $(BUILD_SYSTEM)/base_rules.mk
endif

ifneq ($(host_os), windows)
my_prebuilt_src_file:= $(LOCAL_PATH)/$(LOCAL_SRC_FILES)
else
my_prebuilt_src_file:= $(LOCAL_PATH)\$(LOCAL_SRC_FILES)
endif

ifeq ($(strip $(LOCAL_MODULE_CLASS)),SHARED_LIBRARIES)

ifneq ($(strip $(LOCAL_SONAME_SUFFIX)),)

actual_module := $(addprefix $(dir $(LOCAL_BUILT_MODULE)),$(addsuffix $(LOCAL_SONAME_SUFFIX),$(LOCAL_MODULE)))

$(LOCAL_BUILT_MODULE): PRIVATE_ACTUAL_MODULE := $(actual_module)
$(LOCAL_BUILT_MODULE): $(my_prebuilt_src_file)
	@mkdir -p $(dir $@)
	@cp -rf $< $(PRIVATE_ACTUAL_MODULE)
	@ln -sf $(notdir $(PRIVATE_ACTUAL_MODULE)) $@
else

$(LOCAL_BUILT_MODULE): $(my_prebuilt_src_file)
	+$(transform-prebuilt-to-target)

endif


else

ifeq ($(strip $(LOCAL_MODULE_CLASS)),FOLDER)
$(LOCAL_BUILT_MODULE) : $(my_prebuilt_src_file)
	+$(transform-prebuilt-to-folder)
else
$(LOCAL_BUILT_MODULE) : $(my_prebuilt_src_file)
	+$(transform-prebuilt-to-target)
endif

endif

$(LOCAL_BUILT_MODULE) : $(LOCAL_ADDITIONAL_DEPENDENCIES)

