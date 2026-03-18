ccflags-y += -I$(M)/ \
             -I$(TOP_DIR)/inc \
             -I$(TOP_DIR)/inc/driver \
             -I$(M)/includes/ \
             -I$(M)/src \
             -I$(TOP_DIR)/abl/libc_sec/include \
             -I$(TOP_DIR)/inc/soc/mini_v3 \
             -I$(TOP_DIR)/drivers/firmware/hiss/inc/hiss \
             -I$(DRIVER_KERNEL_DIR)/inc/

ifeq ($(strip $(TARGET_BUILD_TYPE)),debug)
    ccflags-y += -I$(DRIVER_CHIP_KERNEL_DIR)/src/unreleased/serdes_h25
endif

ifneq ($(filter $(PRODUCT),  as31xm1),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
endif

ifeq ($(strip $(TARGET_BUILD_TYPE)),debug)
    EXTRA_CFLAGS += -DCFG_BUILD_DEBUG
endif

ccflags-y += -Wall -Werror -fno-common -Wfloat-equal -Wformat=2 $(WDATE_TIME) -Wcast-align  -Wundef -Wstrict-prototypes
ccflags-y += -Wunused -O2 -Wextra -Wno-unused-parameter -Wno-sign-compare

ifeq ($(use_hitest),kernel_true)
    CC:=hitestwrapper $(CC)
    LD:=hitestwrapper $(LD)
endif

ifneq ($(filter $(PRODUCT),  as31xm1),)
EXTRA_CFLAGS += -DFEATURE_HIGH_IMPEDANCE_ON_SUSPEND
EXTRA_CFLAGS += -DELASTIC_CFG_OFFSET_ENABLE
endif

EXTRA_CFLAGS += -DSNAPSHOT_ENABLE
EXTRA_CFLAGS += -DNOT_SUPPORT_OSNAME

KERNEL_DIR :=$(srctree)
MODULE_NAME := drv_serdes
obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-objs := src/sdk_hilink_common.o \
                       src/sdk_hilink_init.o \
                       src/sdk_hilink_cfg.o \
                       src/hilink_fw_data_h25.o \
                       src/drv_snapshot.o

ifeq ($(strip $(TARGET_BUILD_TYPE)),debug)
    $(MODULE_NAME)-objs += sdk_hilink_dfx.o
endif
