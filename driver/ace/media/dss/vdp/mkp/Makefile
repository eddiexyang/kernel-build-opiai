KERNEL_DIR := $(srctree)
OSTYPE := linux
EXTRA_CFLAGS += -I$(srctree)/include/linux \
	        -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/inc \

# HDMI dir
DSS_DIR := ${TOP_DIR}/ace/media/dss/vdp
HDMI_DIR := $(DSS_DIR)
HDMI_MKP_DIR := $(HDMI_DIR)/mkp
HDMI_MPI_DIR := $(HDMI_DIR)/mpi

# COMMON dir
MB_TOP_INC_DIR := $(TOP_DIR)/ace/media/mediabase/inc
MB_TOP_SRC_DIR := $(TOP_DIR)/ace/media/mediabase/src
MB_INC_EXT_DIR := ${MB_TOP_INC_DIR}/external/mediabase
MB_INC_COMMON_DIR := ${MB_TOP_INC_DIR}/mediabase
MB_SRC_BASE_ARCH_DIR := ${MB_TOP_SRC_DIR}/base/arch/arch11/include

# H8 adapt dir
H8_ADAPT_TOP_DIR := $(MB_INC_COMMON_DIR)/h8_include

HDMI_CHIP := archv200
EXTRA_CFLAGS += -DHDMI_PRODUCT_CHIP_1
EXTRA_CFLAGS += -DCHIP_TYPE_hi3536av100
EXTRA_CFLAGS += -DHDMI_SUPPORT_DUAL_CHANNEL
CTRL_DIR := hal/ctrl/v200
PHY_DIR  := hal/phy/v200
export FPGA=n
export HDMI_DFX_SUPPORT_FLAG=n
export CEC_SUPPORT=n
export FRL_SUPPORT=n
export SCDC_SUPPORT=y
export CONFIG_OT_PROC_SHOW_SUPPORT=y
EXTRA_CFLAGS += -DCONFIG_OT_PROC_SHOW_SUPPORT

####################################################################
# path
####################################################################

MB_INC_PATH := -I${MB_INC_EXT_DIR}
MB_INC_PATH += -I${MB_INC_COMMON_DIR}/include
MB_INC_PATH += -I${MB_INC_COMMON_DIR}/include/adapt
MB_INC_PATH += -I${MB_INC_COMMON_DIR}/base/ext_inc
MB_INC_PATH += -I${MB_INC_COMMON_DIR}/base/adapt
MB_INC_PATH += -I${MB_INC_COMMON_DIR}/base/inner
MB_INC_PATH += -I${MB_INC_COMMON_DIR}/osal
MB_INC_PATH += -I${MB_INC_COMMON_DIR}/sys/ext_inc
MB_INC_PATH += -I${MB_INC_COMMON_DIR}/sys/adapt
MB_INC_PATH += -I${MB_SRC_BASE_ARCH_DIR}
MB_INC_PATH += -I$(TOP_DIR)/inc/driver
MB_INC_PATH += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/include
MB_INC_PATH += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/include/adapt
MB_INC_PATH += -I$(TOP_DIR)/ace/dvpp/inc/dvpp
MB_INC_PATH += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/base/arch/hidvpp/include
MB_INC_PATH += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/base/ext_inc
MB_INC_PATH += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/sys/include
MB_INC_PATH += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/sys/ext_inc
MB_INC_PATH += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/sys/include/adapt

H8_ADAPT_INC_PATH := -I$(H8_ADAPT_TOP_DIR)/include
H8_ADAPT_INC_PATH += -I$(H8_ADAPT_TOP_DIR)/osal/include
H8_ADAPT_INC_PATH += -I$(H8_ADAPT_TOP_DIR)/osal/${OSTYPE}
H8_ADAPT_INC_PATH += -I$(H8_ADAPT_TOP_DIR)/base/ext_inc
H8_ADAPT_INC_PATH += -I$(H8_ADAPT_TOP_DIR)/base/init/include
H8_ADAPT_INC_PATH += -I$(H8_ADAPT_TOP_DIR)/base/mkp/include
H8_ADAPT_INC_PATH += -I$(H8_ADAPT_TOP_DIR)/base/arch/include
H8_ADAPT_INC_PATH += -I$(H8_ADAPT_TOP_DIR)/base/arch/$(HDMI_CHIP)/include
H8_ADAPT_INC_PATH += -I$(H8_ADAPT_TOP_DIR)/sys/include
H8_ADAPT_INC_PATH += -I$(H8_ADAPT_TOP_DIR)/sys/ext_inc
H8_ADAPT_INC_PATH += -I$(H8_ADAPT_TOP_DIR)/sys/init/include
H8_ADAPT_INC_PATH += -I$(H8_ADAPT_TOP_DIR)/sys/mkp/include
H8_ADAPT_INC_PATH += -I$(H8_ADAPT_TOP_DIR)/sys/arch/include
H8_ADAPT_INC_PATH += -I$(H8_ADAPT_TOP_DIR)/sys/arch/$(HDMI_CHIP)/include
H8_ADAPT_INC_PATH += -I$(H8_ADAPT_TOP_DIR)/base/arch/archv200/include/

HDMI_COMMON_PATH := -I$(HDMI_DIR)/include

CUR_DIR := $(PWD)
LINUX_VERSION := osal/v600
HDMI_MKP_INC_PATH := -I$(HDMI_MKP_DIR)
HDMI_MKP_INC_PATH += -I$(HDMI_MKP_DIR)/include
HDMI_MKP_INC_PATH += -I$(HDMI_MKP_DIR)/init/$(HDMI_CHIP)/include
HDMI_MKP_INC_PATH += -I$(HDMI_MKP_DIR)/hal
HDMI_MKP_INC_PATH += -I$(HDMI_MKP_DIR)/$(CTRL_DIR)
HDMI_MKP_INC_PATH += -I$(HDMI_MKP_DIR)/$(PHY_DIR)
HDMI_MKP_INC_PATH += -I$(HDMI_MKP_DIR)/$(CTRL_DIR)/regs
HDMI_MKP_INC_PATH += -I$(HDMI_MKP_DIR)/$(PHY_DIR)/regs
HDMI_MKP_INC_PATH += -I$(HDMI_MKP_DIR)/$(LINUX_VERSION)
HDMI_MKP_INC_PATH += -I$(HDMI_MKP_DIR)/product/$(HDMI_CHIP)
HDMI_MKP_INC_PATH += -I$(HDMI_MKP_DIR)/product/$(HDMI_CHIP)/regs
HDMI_MKP_INC_PATH += -I$(HDMI_MKP_DIR)/dfx
HDMI_MKP_INC_PATH += -I$(DSS_DIR)/dfx

INC_PATH := $(MB_INC_PATH)
INC_PATH += $(H8_ADAPT_INC_PATH)
INC_PATH += $(HDMI_COMMON_PATH)
INC_PATH += $(HDMI_MKP_INC_PATH)


SRCS += $(CTRL_DIR)/regs/hdmi_reg_aon.o \
        $(CTRL_DIR)/regs/hdmi_reg_audio_path.o \
        $(CTRL_DIR)/regs/hdmi_reg_ctrl.o \
        $(CTRL_DIR)/regs/hdmi_reg_tx.o \
        $(CTRL_DIR)/regs/hdmi_reg_video_path.o

SRCS += $(CTRL_DIR)/hdmi_hal_intf.o \
        $(CTRL_DIR)/hdmi_hal_machine.o \
        $(CTRL_DIR)/hdmi_hal_ddc.o \
        $(CTRL_DIR)/hdmi_hal_ncts.o \
        $(CTRL_DIR)/hdmi_hal_ctrl.o

SRCS += product/$(HDMI_CHIP)/regs/hdmi_reg_crg.o \
        product/$(HDMI_CHIP)/hdmi_product_define.o \
        $(PHY_DIR)/regs/hdmi_reg_dphy.o \
        $(PHY_DIR)/hdmi_hal_phy.o

SRCS += init/$(HDMI_CHIP)/src/hdmi_init.o \
        drv_hdmi_infoframe.o \
        drv_hdmi_event.o \
        drv_hdmi_edid.o \
        drv_hdmi_intf.o \
        drv_hdmi_intf_k.o \
        drv_hdmi_common.o \
        drv_hdmi_compatibility.o \
        $(LINUX_VERSION)/hdmi_osal.o

ifeq ($(CONFIG_OT_PROC_SHOW_SUPPORT), y)
SRCS += drv_hdmi_proc.o
endif

ifeq ($(FRL_SUPPORT), y)
EXTRA_CFLAGS += -DHDMI_FRL_SUPPORT
SRCS += $(CTRL_DIR)/regs/hdmi_reg_frl.o \
        $(CTRL_DIR)/hdmi_hal_frl.o \
        drv_hdmi_frl.o \
        drv_hdmi_dfm.o
endif

ifeq ($(SCDC_SUPPORT), y)
EXTRA_CFLAGS += -DHDMI_SCDC_SUPPORT
SRCS += $(CTRL_DIR)/hdmi_hal_scdc.o
endif

ifeq ($(HDR_SUPPORT), y)
EXTRA_CFLAGS += -DHDMI_HDR_SUPPORT
endif

# if debug support, add drv_hdmi_debug.o
EXTRA_CFLAGS += -DHDMI_DEBUG_SUPPORT
SRCS += drv_hdmi_debug.o

#dfx support
ifeq ($(HDMI_DFX_SUPPORT_FLAG),y)
SRCS += dfx/drv_hdmi_timing.o
SRCS += dfx/drv_hdmi_reset.o
SRCS += dfx/drv_hdmi_pattern.o
SRCS += dfx/drv_hdmi_detection.o
SRCS += dfx/drv_hdmi_capture.o
SRCS += dfx/drv_avi_infoframe.o
SRCS += dfx/drv_hdmi_hotplug.o
endif

EXTRA_CFLAGS += -DHDMI_DEFUALT_CALLBACK_SUPPORT

ifeq ($(FPGA),y)
EXTRA_CFLAGS += -DHDMI_FPGA_SUPPORT
endif

EXTRA_CFLAGS += -Wall -c -O2 -funsigned-char
EXTRA_CFLAGS += -Wno-error=date-time
EXTRA_CFLAGS += -I. $(INC_PATH) -lpthread
EXTRA_CFLAGS += -D$(PRODUCT)

#export hdmi functional support switch
EXTRA_CFLAGS += -DDVI_SUPPORT
#EXTRA_CFLAGS += -DHDMI_LOG_SUPPORT

EXTRA_CFLAGS += -I$(PWD)

EXTRA_CFLAGS += -Wextra -Wfloat-equal -Wnon-virtual-dtor -Wunused -Wtrampolines
EXTRA_CFLAGS += -fstrong-eval-order -fvisibility=hidden
EXTRA_CFLAGS += -Wswitch-default -Wdelete-non-virtual-dtor -Wundef -Wdate-time -Wconversion -Wshadow -Wvla
EXTRA_CFLAGS += -Wformat=2
EXTRA_CFLAGS += -freg-struct-return
EXTRA_CFLAGS += -pipe
EXTRA_CFLAGS += -Wcast-align -Wcast-qual -Woverloaded-virtual
EXTRA_CFLAGS += -fno-common
EXTRA_CFLAGS += -Wstrict-prototypes -Wall

obj-m += drv_hdmi.o
drv_hdmi-objs = $(SRCS)
