ifeq ($(TOP_DIR),)
KERNEL_DIR :=$(srctree)
EXTRA_CFLAGS += -I$(PWD)/driver/src/kernel/eeprom
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_plat/common
EXTRA_CFLAGS += -I$(PWD)/inc


ifeq ($(TARGET_PRODUCT), mini)
EXTRA_CFLAGS += -DUSE_MINI_PLATFORM -DENV_SMMU_SUPPORT -DDVPP_WAKELOCK
endif

obj-m   += drv_e2prom.o
drv_e2prom-objs := eeprom_M24256.o
else

EXTRA_CFLAGS += -I$(DRIVER_CHIP_KERNEL_DIR)/src/eeprom
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common
EXTRA_CFLAGS += -I$(TOP_DIR)/inc

ifneq ($(filter $(PRODUCT),  ascend310 ascend310rc ascend610 ascend310p),)
EXTRA_CFLAGS += -DUSE_MINI_PLATFORM -DENV_SMMU_SUPPORT -DDVPP_WAKELOCK
endif

obj-m   += drv_e2prom.o
drv_e2prom-objs := eeprom_M24256.o

endif
