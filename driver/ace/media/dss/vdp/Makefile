VDP_PATH        := $(TOP_DIR)/ace/media/dss/vdp
OT_ARCH         ?= archv200
MPP_PATH := $(TOP_DIR)/ace/media/mediabase/inc/mediabase/h8_include
MB_DIR := $(TOP_DIR)/ace/media/mediabase
MB_INC_EXT_DIR := ${MB_DIR}/inc/external/mediabase
MB_COMMON_DIR := ${MB_DIR}/inc/mediabase
MB_BASE_DIR := ${MB_COMMON_DIR}/base
MB_OSAL_DIR := ${MB_COMMON_DIR}/osal
MB_SYS_DIR := ${MB_COMMON_DIR}/sys
MB_BASE_ARCH_DIR := ${MB_DIR}/src/base/arch/arch11/include
MB_BASE_EXT_DIR := ${MB_DIR}/src/base/ext_inc

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
MB_INC_PATH += -I${MB_BASE_EXT_DIR}

COMMON_INCLUDE := ${MB_INC_PATH}
COMMON_INCLUDE += -I$(MPP_PATH)/include
COMMON_INCLUDE += -I$(MPP_PATH)/osal/include
COMMON_INCLUDE += -I$(MPP_PATH)/base/include
COMMON_INCLUDE += -I$(MPP_PATH)/base/ext_inc
COMMON_INCLUDE += -I$(MPP_PATH)/base/arch/$(OT_ARCH)/include
COMMON_INCLUDE += -I$(MPP_PATH)/sys/include
COMMON_INCLUDE += -I$(MPP_PATH)/sys/ext_inc
COMMON_INCLUDE += -I$(MPP_PATH)/sys/arch/$(OT_ARCH)/include
COMMON_INCLUDE += -I$(TOP_DIR)/libc_sec/include
COMMON_INCLUDE += -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/inc
#*******************************************************************************
#********************* Set the source code and include path ********************
# Head file path variable's

EXTRA_CFLAGS += $(COMMON_INCLUDE)

EXTRA_CFLAGS += -I$(VDP_PATH)/gfbg/include
EXTRA_CFLAGS += -I$(VDP_PATH)/gfbg/src
EXTRA_CFLAGS += -I$(VDP_PATH)/gfbg/src/init
EXTRA_CFLAGS += -I$(VDP_PATH)/gfbg/drv/adp
EXTRA_CFLAGS += -I$(VDP_PATH)/gfbg/drv/include

EXTRA_CFLAGS += -I$(VDP_PATH)/vo/ext_inc
EXTRA_CFLAGS += -I$(VDP_PATH)/tde/drv/include
EXTRA_CFLAGS += -I$(VDP_PATH)/dfx

EXTRA_CFLAGS += -DCONFIG_COMPRESS_ECONOMIZE_MEMERY

EXTRA_CFLAGS += -DCONFIG_GFBG_LOW_DELAY_SUPPORT
EXTRA_CFLAGS += -DCONFIG_TDE_GFBG_COMPRESS_V2
EXTRA_CFLAGS += -DCONFIG_GFBG_SMART_RECT_V2
EXTRA_CFLAGS += -DCONFIG_GFBG_G1_SUPPORT_ZME
EXTRA_CFLAGS += -DCONFIG_TDE_CLUT_RECT_V2
EXTRA_CFLAGS += -DCONFIG_OT_VO_FB_SEPARATE
EXTRA_CFLAGS += -DCONFIG_GFBG_G1_4K_SUPPORT
#***************************************************************************#


#********************* List all the release file *******************************

ifeq ($(OT_GDB),y)
EXTRA_CFLAGS += -g
endif

EXTRA_CFLAGS += $(CFG_INC)
EXTRA_CFLAGS += -Wno-error=date-time

-include $(VDP_PATH)/gfbg/src/Makefile
-include $(VDP_PATH)/gfbg/drv/adp/Makefile

SRCS := src/gfbg_main.c \
       src/gfbg_proc.c \
       drv/adp/gfbg_vou_drv.c \
       drv/adp/gfbg_blit.c \
       drv/adp/gfbg_vou_graphics.c \
       drv/adp/gfbg_graphics_drv.c \
       drv/adp/gfbg_graphic_hal.c \
       drv/adp/gfbg_comm.c \
       drv/adp/gfbg_rotate.c \
       src/init/linux/gfbg_init.c \
       drv/adp/gfbg_rotate_tde.c

MOD_NAME := mod_fb
SRCS_O := $(SRCS)
TARGET := ascend_vdp_hifb

$(TARGET)-objs = $(SRCS:%.c=%.o)
obj-m := $(TARGET).o
