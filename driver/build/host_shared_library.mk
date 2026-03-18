my_skip_this_target :=
ifneq ($(LLT_TEST)$(SKIP_HOST),)
  my_skip_this_target := true
endif

ifneq (true,$(my_skip_this_target))

ifeq ($(strip $(LOCAL_MODULE_CLASS)),)
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
endif

ifeq ($(strip $(LOCAL_MODULE_SUFFIX)),)
LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
endif

ifneq ($(LOCAL_USE_COMPILER),)
my_prefix := $(LOCAL_USE_COMPILER)_
else
my_prefix := $(HOST_DEFAULT_COMPILER)_
endif

LOCAL_CFLAGS += -fPIC

include $(BUILD_SYSTEM)/binary.mk

$(LOCAL_BUILT_MODULE): $(all_libraries) $(all_objects)
	$(transform-o-to-shared-lib)

endif
