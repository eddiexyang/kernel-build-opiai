CBBPATH := $(TOP_DIR)/ace/media/camera/camerav2
BASE_PATH := $(TOP_DIR)/ace/media/mediabase/src
BASE_INC_PATH := $(TOP_DIR)/ace/media/mediabase/inc
COMM_PATH := $(TOP_DIR)/ace/media/camera/camerav2/common


ccflags-y += -I$(CBBPATH)/mipi_rx
ccflags-y += -I$(CBBPATH)/mipi_rx/csi2_lvds_hal
ccflags-y += -I$(CBBPATH)/mipi_rx/slvs_ec_hal

ccflags-y +=  -I$(BASE_INC_PATH)/external/mediabase \
              -I$(BASE_INC_PATH)/mediabase/include \
              -I$(BASE_INC_PATH)/mediabase/vi \
              -I$(BASE_INC_PATH)/mediabase/vpss \
              -I$(BASE_INC_PATH)/mediabase/include/adapt \
              -I$(BASE_INC_PATH)/mediabase/base/inner \
              -I$(BASE_INC_PATH)/mediabase/base/ext_inc \
              -I$(BASE_INC_PATH)/mediabase/base/adapt \
              -I$(BASE_PATH)/base/arch/arch11/include \
              -I$(BASE_PATH)/base/arch/include \
              -I$(BASE_PATH)/base/mkp/include \
              -I$(BASE_PATH)/base/profiling \
              -I$(BASE_INC_PATH)/mediabase/osal \
              -I$(BASE_PATH)/osal/linux/kernel/himedia \
              -I$(BASE_PATH)/osal/linux/kernel \
              -I$(BASE_INC_PATH)/mediabase/sys/adapt \
              -I$(BASE_INC_PATH)/mediabase/sys/ext_inc \
              -I$(BASE_INC_PATH)/mediabase/sys/inner \
              -I$(BASE_PATH)/sys/arch/include \
              -I$(BASE_PATH)/sys/arch/arch11/include \
              -I$(BASE_PATH)/sys/mkp/include \
              -I$(CBBPATH)/vi/include/adapt \
              -I$(CBBPATH)/vi/include \
              -I$(CBBPATH)/vi/ext_inc \
              -I$(CBBPATH)/common/include \
              -I$(TOP_DIR)/ace/media/inc/media/camera/camerav2 \
              -I$(TOP_DIR)/inc/driver   \

ccflags-y += -I$(TOP_DIR)/libc_sec/include

ccflags-y += -Wno-error=date-time
ccflags-y += -Wno-date-time
ccflags-y += -Wno-error=incompatible-pointer-types
EXTRA_CFLAGS += -DBUILD_MODULE_ISP

obj-m += drv_mipi_rx.o
drv_mipi_rx-objs := mipi_rx_entry.o \
                    mipi_rx_proc.o \
                    mipi_rx_init.o \
                    mipi_rx_operate.o \
                    csi2_lvds_hal/mipi_rx_hal.o \
                    slvs_ec_hal/slvs_hal.o