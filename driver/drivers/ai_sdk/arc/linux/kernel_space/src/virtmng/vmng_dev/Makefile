ifeq ($(TOP_DIR),)

else # for cmake

EXTRA_CFLAGS += -fstack-protector-all

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_public
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver/
EXTRA_CFLAGS += -I${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/inc/dbl
EXTRA_CFLAGS += -DVIRTMNG_DEV

	ifneq ($(filter $(PRODUCT),  ascend910 ascend910B ascend920esl ascend920emu),)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
		ifneq ($(filter $(PRODUCT),  ascend910B ascend920esl ascend920emu ),)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
		endif
	else ifeq ($(PRODUCT),ascend310p)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
	else ifeq ($(PRODUCT),ascend310Brc)
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3
		EXTRA_CFLAGS += -DCFG_FEATURE_SOC_VIRTMNG
	else
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV1
	endif

endif

ifneq ($(PRODUCT),as31xm1)
	EXTRA_CFLAGS += -DCFG_SURPPORT_PCIE_HOST_DEVICE_COMM
endif

ifneq ($(filter $(PRODUCT),  ascend910B ascend920esl ascend920emu ),)
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_dev/soc/adapt/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_dev/soc/cloud_v2/
else ifeq ($(PRODUCT),ascend310p)
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_dev/soc/mini_v2/
else ifeq ($(PRODUCT),ascend310Brc)
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_dev/soc/adapt/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_dev/soc/mini_v3/
else
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_dev/soc/default/
endif
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_dev/soc/common
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_dev/soc/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/virtmng/vmng_dev/

ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers -Wno-implicit-fallthrough

obj-m += ascend_virtmng_dev.o
ascend_virtmng_dev-objs := virtmngdev_pci.o
ascend_virtmng_dev-objs += virtmngdev_proc_fs.o

ascend_virtmng_dev-objs += soc/virtmngdev_res_common.o
ascend_virtmng_dev-objs += soc/virtmngdev_element.o
ascend_virtmng_dev-objs += soc/common/virtmngdev_common_resource.o

ascend_virtmng_dev-objs += soc/common/virtmngdev_res_mng.o
ascend_virtmng_dev-objs += soc/common/virtmngdev_client_online.o

ifneq ($(filter $(PRODUCT),  ascend910B ascend920esl ascend920emu ascend310Brc),)
ascend_virtmng_dev-objs += soc/adapt/virtmngdev_resource_register.o
ascend_virtmng_dev-objs += soc/adapt/virtmngdev_resource.o
ascend_virtmng_dev-objs += soc/adapt/virtmngdev_mia_cfg.o
ascend_virtmng_dev-objs += soc/adapt/virtmngdev_memory.o
ascend_virtmng_dev-objs += soc/adapt/virtmngdev_ts.o
EXTRA_CFLAGS += -DCFG_FEATURE_VIRTMNG_SUPPORT_UDA
else ifeq ($(PRODUCT),ascend310p)
ascend_virtmng_dev-objs += soc/mini_v2/virtmngdev_resource.o
ascend_virtmng_dev-objs += soc/mini_v2/virtmngdev_memory.o
else
ascend_virtmng_dev-objs += soc/default/virtmngdev_resource.o
endif
