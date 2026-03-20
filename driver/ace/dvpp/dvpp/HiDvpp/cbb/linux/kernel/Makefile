ARCDIR := hidvpp
BASE_PATH := $(TOP_DIR)/ace/media/mediabase/src
BASE_INC_PATH := $(TOP_DIR)/ace/media/mediabase/inc

ifeq ($(PRODUCT), helper310p)
ifeq ($(filter $(HELPER_KERNAL), openeuler),)
    EXTRA_CFLAGS += -DSHARE_POOL_GROUP_ID
endif
    EXTRA_CFLAGS += -DDVPP_HLP
endif

ifeq ($(PRODUCT), as31xm1)
EXTRA_CFLAGS += -DDVPP_MINI_V2_MDC
endif

EXTRA_CFLAGS += -I$(BASE_INC_PATH)/external/mediabase \
        -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/include \
        -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/include/adapt \
        -I$(TOP_DIR)/ace/dvpp/inc/dvpp \
        -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/base/ext_inc \
        -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/base/arch/hidvpp/include \
        -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/linux/kernel/himedia \
        -I$(BASE_INC_PATH)/mediabase/include \
        -I$(BASE_INC_PATH)/mediabase/include/adapt \
        -I$(BASE_INC_PATH)/mediabase/osal \
        -I$(BASE_PATH)/osal/linux/kernel/himedia \
        -I$(BASE_PATH)/osal/linux/kernel \
        -I$(BASE_PATH)/base/arch/$(ARCDIR)/include \
        -I$(TOP_DIR)/libc_sec/include \
        -I$(TOP_DIR)/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-gnu-5.4/aarch64-linux-gnu/include \
        -I$(TOP_DIR)/kernel/linux-source/arch/arm/include \
        -I$(TOP_DIR)/kernel/linux-source/arch/arm64/include \
        -I$(TOP_DIR)/ace/media/inc/media/camera/camerav2 \
        -I$(TOP_DIR)/ace/media/inc/media/camera/camerav2/adapt

EXTRA_CFLAGS += -Wno-unused-parameter
EXTRA_CFLAGS += -fstack-protector-all
EXTRA_CFLAGS += -fno-common
EXTRA_CFLAGS += -O2
EXTRA_CFLAGS += -Wextra
EXTRA_CFLAGS += -Wformat=2
EXTRA_CFLAGS += -Wfloat-equal
EXTRA_CFLAGS += -Wdate-time
EXTRA_CFLAGS += -Wshadow
EXTRA_CFLAGS += -Wunused
EXTRA_CFLAGS += -Wstrict-prototypes
EXTRA_CFLAGS += -DENABLE_ADVSMARTP
EXTRA_CFLAGS += -DBUILD_DRV_OSAL

obj-m   += drv_osal.o
drv_osal-objs :=
drv_osal-objs += osal.o osal_addr.o osal_atomic.o osal_barrier.o osal_of.o \
                 osal_debug.o osal_cache.o osal_device.o osal_fileops.o osal_property.o \
                 osal_interrupt.o osal_math.o osal_mutex.o osal_acpi.o osal_bootdot.o \
                 osal_platform.o osal_proc.o osal_schedule.o osal_semaphore.o \
                 osal_spinlock.o osal_string.o osal_task.o osal_timer.o \
                 osal_vmalloc.o osal_wait.o osal_workqueue.o himedia/base.o \
                 himedia/himedia.o
