
# Used to force goals to build.  Only use for conditionally defined goals.
.PHONY: FORCE
FORCE:

#set target product name,default for mini
ifneq ($(product),)
    TARGET_PRODUCT := $(product)
    ifeq ($(product),cloud)
        PRODUCT := ascend910
    endif
    ifeq ($(product),mini)
        ifeq ($(chip_id),hi1910p)
            PRODUCT := ascend310p
        else
            PRODUCT := ascend310
        endif
    endif
    ifneq ($(filter $(product),ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu helper310p as31xm1),)
        TARGET_PRODUCT := mini
        PRODUCT := $(product)
    endif
else
    TARGET_PRODUCT := mini
    PRODUCT := ascend310
endif

export TARGET_PRODUCT
export PRODUCT


#set simultaneouse num
JOBS_N := $(shell cat /proc/cpuinfo /proc/cpuinfo | grep ^processor | wc -l)

#set host internal compile config
BUILD_HOST_STATIC_LIBRARY := $(BUILD_SYSTEM)/host_static_library.mk
BUILD_HOST_SHARED_LIBRARY := $(BUILD_SYSTEM)/host_shared_library.mk
BUILD_HOST_EXECUTABLE := $(BUILD_SYSTEM)/host_executable.mk
BUILD_HOST_PREBUILT := $(BUILD_SYSTEM)/host_prebuilt.mk
BUILD_HOST_KO := $(BUILD_SYSTEM)/host_ko.mk

#set device internal compile config
BUILD_DEVICE_KO := $(BUILD_SYSTEM)/device_ko.mk

CLEAR_VARS := $(BUILD_SYSTEM)/clear_vars.mk

#include ccache tools
include $(BUILD_SYSTEM)/ccache.mk

#include compiler config
include $(BUILD_SYSTEM)/compiler_config.mk

#set module suffix
TARGET_EXECUTABLE_SUFFIX :=
TARGET_SHLIB_SUFFIX := .so
STATIC_LIB_SUFFIX := .a

#set kernel src dir
KERNEL_SRC_DIR := kernel/$(LINUX_VERSION)

#set output path
OUT_ROOT := $(TOPDIR)out
OUT_TOOLS_ROOT := $(TOPDIR)out/tools
#set host output path
HOST_OUT_ROOT := $(OUT_ROOT)/host
HOST_OUT_INTERMEDIATES := $(HOST_OUT_ROOT)/obj
HOST_OUT_INTERMEDIATE_LIBRARIES := $(HOST_OUT_INTERMEDIATES)/lib
HOST_OUT_THIRD_PARTY_LIBS := $(HOST_OUT_INTERMEDIATES)/THIRD_PARTY_LIBS
#set device output path
DEVICE_OUT_ROOT := $(OUT_ROOT)/device
DEVICE_OUT_INTERMEDIATES := $(DEVICE_OUT_ROOT)/obj
DEVICE_OUT_INTERMEDIATE_LIBRARIES := $(DEVICE_OUT_INTERMEDIATES)/lib
DEVICE_OUT_THIRD_PARTY_LIBS := $(DEVICE_OUT_INTERMEDIATES)/THIRD_PARTY_LIBS

RELEASE_IMAGES_OUT := $(OUT_ROOT)/release_imags
PROTO_OUT_ROOT := $(OUT_ROOT)/proto

ifneq ($(O),)
RELEASE_DIR := $(O)
else
RELEASE_DIR := $(RELEASE_IMAGES_OUT)
endif

#set host driver ko output dir
HOST_KO_OBJ_OUT := $(OUT_ROOT)/host/ko_obj
HOST_KO_INSTALLED_OUT := $(OUT_ROOT)/host/ko

#set host kernel deb pkg dir
HOST_KERNEL_DEB_PKG_OUT := $(HOST_OUT_ROOT)/debian-packages

#set device driver ko output dir
DEVICE_KO_OBJ_OUT := $(OUT_ROOT)/device/ko_obj
DEVICE_KO_INSTALLED_OUT := $(OUT_ROOT)/device/ko

#set device kernel deb pkg dir
DEVICE_KERNEL_DEB_PKG_OUT := $(DEVICE_OUT_ROOT)/debian-packages

#set feature.mk path
FEATURE_MK_PATH := $(PWD)/config/feature_config/feature.mk

#set WDATE_TIME
WDATE_TIME := -Wdate-time
