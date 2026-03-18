MAKE_PRODUCT := $(PRODUCT)
ifeq ($(TARGET_BUILD_TYPE),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

ccflags-y += -Wall -Werror

DRIVER_SRC_BASE_DIR := $(DRIVER_KERNEL_DIR)/src

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(TOP_DIR)/libc_sec/include
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver

EXTRA_CFLAGS += -I$(DRIVER_SRC_BASE_DIR)/rms/trs_drv/inc

obj-m += ascend_trs_id_pool.o
ascend_trs_id_pool-objs := id_pool.o

ifneq ($(filter $(PRODUCT), as31xm1), )
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
endif
