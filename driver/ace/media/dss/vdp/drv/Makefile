DSS_DIR := $(TOP_DIR)/ace/media/dss/vdp
DRIVER_DIR := $(DSS_DIR)/drv

#ifneq ($(OT_MAKE_DEBUG),)
#EXTRA_CFLAGS +=-D$(OT_MAKE_DEBUG)
#endif

SIXTYFOUR_SYS_STATUS ?= y
ifeq ($(SIXTYFOUR_SYS_STATUS), y)
EXTRA_CFLAGS += -DSIXFOUR_SYS_STATUS=1
else
EXTRA_CFLAGS += -DSIXFOUR_SYS_STATUS=0
endif

OT_ARCH ?= archv200
CONFIG_OT_PROC_SHOW_SUPPORT ?= y

MPP_PATH := $(TOP_DIR)/ace/media/mediabase/inc/mediabase/h8_include
MB_DIR := $(TOP_DIR)/ace/media/mediabase
MB_INC_EXT_DIR := ${MB_DIR}/inc/external/mediabase
MB_COMMON_DIR := ${MB_DIR}/inc/mediabase
MB_BASE_DIR := ${MB_COMMON_DIR}/base
MB_OSAL_DIR := ${MB_COMMON_DIR}/osal
MB_SYS_DIR := ${MB_COMMON_DIR}/sys
MB_BASE_ARCH_DIR := ${MB_DIR}/src/base/arch/arch11/include

MB_INC_PATH := -I${MB_INC_EXT_DIR}
MB_INC_PATH += -I${MB_COMMON_DIR}/include
MB_INC_PATH += -I${MB_COMMON_DIR}/include/adapt
MB_INC_PATH += -I${MB_BASE_DIR}/ext_inc
MB_INC_PATH += -I${MB_BASE_DIR}/adapt
MB_INC_PATH += -I${MB_BASE_DIR}/inner
MB_INC_PATH += -I${MB_OSAL_DIR}
MB_INC_PATH += -I${MB_SYS_DIR}/ext_inc
MB_INC_PATH += -I${MB_SYS_DIR}/adapt
MB_INC_PATH += -I${MB_BASE_ARCH_DIR}

COMMON_INCLUDE := ${MB_INC_PATH}
COMMON_INCLUDE += -I$(MPP_PATH)/include
COMMON_INCLUDE += -I$(MPP_PATH)/osal/include
COMMON_INCLUDE += -I$(MPP_PATH)/osal/include
COMMON_INCLUDE += -I$(MPP_PATH)/base/include
COMMON_INCLUDE += -I$(MPP_PATH)/base/ext_inc
COMMON_INCLUDE += -I$(MPP_PATH)/base/arch/$(OT_ARCH)/include
COMMON_INCLUDE += -I$(MPP_PATH)/sys/include
COMMON_INCLUDE += -I$(MPP_PATH)/sys/ext_inc
COMMON_INCLUDE += -I$(TOP_DIR)/ace/media/dss/vdp/dfx
COMMON_INCLUDE += -I$(DSS_DIR)/include
COMMON_INCLUDE += -I$(DRIVER_DIR)/init/include
COMMON_INCLUDE += -I$(TOP_DIR)/libc_sec/include
COMMON_INCLUDE += -I$(TOP_DIR)/inc/driver
COMMON_INCLUDE += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/include
COMMON_INCLUDE += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/include/adapt
COMMON_INCLUDE += -I$(TOP_DIR)/ace/dvpp/inc/dvpp
COMMON_INCLUDE += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/base/arch/hidvpp/include
COMMON_INCLUDE += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/base/ext_inc

DRV_CFLAGS := -Wall -fsigned-char -Wno-error=date-time
DRV_CFLAGS += -Wfloat-equal -Wformat=2

MPP_CFLAGS := -Wall -fsigned-char
MPP_CFLAGS += -fno-common
DRV_CFLAGS += --warn-stack-usage=800
DRV_CFLAGS += $(COMMON_INCLUDE)
MPP_CFLAGS += $(COMMON_INCLUDE)

EXTRA_CFLAGS += -Wno-error=date-time
EXTRA_CFLAGS += -Wcast-align
EXTRA_CFLAGS += -Wtrampolines
EXTRA_CFLAGS += -Wunused
EXTRA_CFLAGS += -Wconversion
EXTRA_CFLAGS += -Wvla
EXTRA_CFLAGS += -Wswitch-default
EXTRA_CFLAGS += -Wextra
EXTRA_CFLAGS += -Wshadow
EXTRA_CFLAGS += -Wcast-qual
EXTRA_CFLAGS += -freg-struct-return
EXTRA_CFLAGS += -fstrong-eval-order
EXTRA_CFLAGS += -fvisibility=hidden
EXTRA_CFLAGS += $(MKP_CFLAGS) $(DRV_CFLAGS)
EXTRA_CFLAGS += -D$(OT_ARCH)
ifeq ($(CONFIG_OT_PROC_SHOW_SUPPORT),y)
EXTRA_CFLAGS += -DCONFIG_OT_PROC_SHOW_SUPPORT
endif
CHIP_TYPE=CHIP_TYPE_$(OT_ARCH)
KBUILD_EXTRA_SYMBOLS += $(OT_KBUILD_EXTRA_SYMBOLS)

EXTRA_CFLAGS += -D$(CHIP_TYPE)

EXTRA_CFLAGS += -DOT_TDE_MEMCOUNT_SUPPORT=0
EXTRA_CFLAGS += -DOT_TDE_BUFFER=0x20000
EXTRA_CFLAGS += -DTDE_LOWPOWER
EXTRA_CFLAGS += -DENV_SMMU_SUPPORT
EXTRA_CFLAGS += -DCONFIG_GFBG_LOW_DELAY_SUPPORT
EXTRA_CFLAGS += -DCONFIG_TDE_GFBG_COMPRESS_V2
EXTRA_CFLAGS += -DCONFIG_TDE_ROTATE_SUPPORT
EXTRA_CFLAGS += -DCONFIG_TDE_DMA_CORNER_V2
EXTRA_CFLAGS += -DCONFIG_TDE_DRD_LINE_SUPPORT

EXTRA_CFLAGS += -DCONFIG_COMPRESS_ECONOMIZE_MEMERY

ifeq ($(PRODUCT), ascend320esl)
       EXTRA_CFLAGS += -DUSE_ESL
endif

EXTRA_CFLAGS += -I$(DRIVER_DIR)/include
EXTRA_CFLAGS += -I$(DSS_DIR)/api/tde/include
EXTRA_CFLAGS += -I$(DRIVER_DIR)/tde/include
EXTRA_CFLAGS += -I$(DRIVER_DIR)/tde/adp/tde_v2_0
EXTRA_CFLAGS += -I$(DRIVER_DIR)/tde/src

SRCS += tde/src/tde_osr.c\
       tde/adp/tde_v2_0/tde_hal_k.c \
       tde/adp/tde_v2_0/tde_osictl_k.c \
       tde/src/tde_osilist.c \
       tde/src/wmalloc.c \
       tde/src/tde_ioctl.c \
       tde/src/tde_handle.c \
       tde/adp/tde_v2_0/tde_adp.c

ifdef CONFIG_OT_PROC_SHOW_SUPPORT
SRCS += tde/src/tde_proc.c
endif

INIT_FILE := init/tde_init.c

MOD_NAME := mod_tde
SRCS_O := $(SRCS)
SRCS += $(INIT_FILE)
TARGET := drv_tde


$(TARGET)-objs := $(SRCS:%.c=%.o)
obj-m := $(TARGET).o
