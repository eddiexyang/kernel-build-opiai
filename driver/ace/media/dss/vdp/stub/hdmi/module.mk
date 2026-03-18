LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/Makefile
include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \

LOCAL_LDFLAGS := -Wl,-z,relro
LOCAL_LDFLAGS += -Wl,-z,now
LOCAL_LDFLAGS += -Wl,-z,noexecstack
LOCAL_LDFLAGS += -fvisibility=hidden

LOCAL_CFLAGS := -O2
LOCAL_CFLAGS += -g
LOCAL_CFLAGS += -fpie
LOCAL_CFLAGS += -fstack-protector-all
LOCAL_CFLAGS += -fpic

LOCAL_SRC_FILES := \
    ../../../../out/hdmi/lib64/stub/hi_mpi_hdmi_stub.c

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../../../../inc/vdp

LOCAL_MODULE := stub/libacl_hdmi_mpi


include $(BUILD_SHARED_LIBRARY)
