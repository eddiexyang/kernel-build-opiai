
CBBPATH := $(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb
PNGDPATH := $(CBBPATH)/pngd
EXTRA_CFLAGS += -DUSE_DVPP_FWK
MEDIABASE_INC_PATH := $(TOP_DIR)/ace/media/mediabase/inc
MEDIABASE_SRC_PATH := $(TOP_DIR)/ace/media/mediabase/src

EXTRA_CFLAGS += -I$(MEDIABASE_INC_PATH)/mediabase/include \
                -I$(MEDIABASE_INC_PATH)/mediabase/include/adapt \
                -I$(MEDIABASE_INC_PATH)/mediabase/osal \
                -I$(MEDIABASE_SRC_PATH)/osal/linux/kernel/himedia \
                -I$(MEDIABASE_INC_PATH)/mediabase/base \
                -I$(MEDIABASE_INC_PATH)/mediabase/base/ext_inc \
                -I$(MEDIABASE_INC_PATH)/mediabase/base/adapt \
                -I$(MEDIABASE_INC_PATH)/mediabase/base/inner \
                -I$(MEDIABASE_SRC_PATH)/base/arch/include \
                -I$(MEDIABASE_SRC_PATH)/base/arch/hidvpp/include \
                -I$(MEDIABASE_SRC_PATH)/base/mkp/include \
                -I$(MEDIABASE_SRC_PATH)/base/profiling \
                -I$(MEDIABASE_INC_PATH)/mediabase/sys \
                -I$(MEDIABASE_INC_PATH)/mediabase/sys/ext_inc \
                -I$(MEDIABASE_INC_PATH)/mediabase/sys/adapt \
                -I$(MEDIABASE_INC_PATH)/mediabase/sys/inner \
                -I$(MEDIABASE_SRC_PATH)/sys/arch/include \
                -I$(MEDIABASE_SRC_PATH)/sys/arch/hidvpp \
                -I$(MEDIABASE_SRC_PATH)/sys/mkp/include \
                -I$(MEDIABASE_SRC_PATH)/sys/acpi_common \
                -I$(MEDIABASE_SRC_PATH)/sys/dts

EXTRA_CFLAGS += -I$(CBBPATH)/chnl/mkp/include \
		-I$(CBBPATH)/drv_dvpp_comm/include \
		-I$(TOP_DIR)/abl/libc_sec/include \
		-I$(TOP_DIR)/inc/driver \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/inc \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/drv_devmng/drv_devmng_inc \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/tsdrv/ts_drv/ts_drv_common \
		-I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/tsdrv/ts_platform/ts_platform_device/ascend910 \
		-I$(TOP_DIR)/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-gnu-5.4/aarch64-linux-gnu/include \
		-I$(TOP_DIR)/kernel/linux-source/arch/arm/include \
		-I$(TOP_DIR)/kernel/linux-source/arch/arm64/include \
        -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/inc

EXTRA_CFLAGS += \
        -I$(PNGDPATH)/arch/include \
        -I$(PNGDPATH)/include/adapt \
        -I$(PNGDPATH)/include/inner \
        -I$(PNGDPATH)/mkp/include \
        -I$(PNGDPATH)/ext_inc \
        -I$(TOP_DIR)/inc/dvpp \
        -I$(TOP_DIR)/ace/dvpp/inc/dvpp \
        -I$(TOP_DIR)/inc/external/dvpp \
        -I$(TOP_DIR)/ace/dvpp/inc/external/dvpp

EXTRA_CFLAGS += -Wno-error=date-time
EXTRA_CFLAGS += -Wno-date-time
EXTRA_CFLAGS += -Wno-error=incompatible-pointer-types
EXTRA_CFLAGS += -DENABLE_ADVSMARTP
EXTRA_CFLAGS += $(VFMW_CFLAGS)
EXTRA_CFLAGS += -DENV_SMMU_SUPPORT

ifeq ($(PRODUCT), ascend310p)
EXTRA_CFLAGS += -DDVPP_MINI
endif

ifeq ($(PRODUCT), helper310p)
EXTRA_CFLAGS += -DDVPP_MINI
endif

ifeq ($(PRODUCT), ascend920esl)
EXTRA_CFLAGS += -DDVPP_ML
endif

ifeq ($(PRODUCT), ascend920emu)
EXTRA_CFLAGS += -DDVPP_ML
endif

ifeq ($(PRODUCT), ascend910B)
EXTRA_CFLAGS += -DDVPP_ML
endif

ifeq ($(PRODUCT), ascend610)
EXTRA_CFLAGS += -DDVPP_MDC
EXTRA_CFLAGS += -DDVPP_PG1
endif

obj-m   += drv_pngd.o
obj-m   += hi_pngd.o

hi_pngd-objs := mkp/src/pngd.o mkp/src/pngd_buf.o mkp/src/pngd_ext.o mkp/src/pngd_intf.o
hi_pngd-objs += mkp/src/pngd_proc.o
drv_pngd-objs := hi_pngd.o pngd_init.o
