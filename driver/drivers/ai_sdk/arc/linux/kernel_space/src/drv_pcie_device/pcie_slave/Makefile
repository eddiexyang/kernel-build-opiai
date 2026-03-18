$(info pcie_slave TOP_DIR = $(TOP_DIR) ......)

ifeq ($(TOP_DIR),)
KERNEL_DIR := $(srctree)

EXTRA_CFLAGS += -fstack-protector-all
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc/
EXTRA_CFLAGS += -I$(PWD)/inc/driver/
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/devdrv/common_h/
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/devdrv/pcie_slave/unit/msg/
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/devdrv/pcie_slave/interfaces/
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/devdrv/pcie_slave/soc/
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/devdrv/pcie_slave/unit/
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/devdrv/pcie_slave/ctrls/

ifeq ($(ENABLE_ASAN),true)
    EXTRA_CFLAGS += -DCFG_ENABLE_ASAN
endif

ifeq ($(TARGET_PRODUCT),mini)
	ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
		EXTRA_CFLAGS += -I$(PWD)/inc/soc/cloud_v1/
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/devdrv/pcie_slave/soc/mdc_v1/
		EXTRA_CFLAGS += -DCFG_FEATURE_P2P
	else
		EXTRA_CFLAGS += -I$(PWD)/inc/soc/mini_v1/
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/devdrv/pcie_slave/soc/mini_v1
		EXTRA_CFLAGS += -DCFG_FEATURE_PM
	endif
endif

ifeq ($(TARGET_PRODUCT),mdc)
	ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
		EXTRA_CFLAGS += -I$(PWD)/inc/soc/cloud_v1/
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/devdrv/pcie_slave/soc/mdc_v1/
		EXTRA_CFLAGS += -I$(PWD)/drivers/src/common/
		EXTRA_CFLAGS += -DCFG_FEATURE_P2P
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V2
		EXTRA_CFLAGS += -DCFG_FEATURE_SUPPORT_CPUDOMAIN
	else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
		EXTRA_CFLAGS += -I$(PWD)/inc/soc/cloud_v1/
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/devdrv/pcie_slave/soc/mdc_v1/
		EXTRA_CFLAGS += -I$(PWD)/drivers/src/common/
		EXTRA_CFLAGS += -DCFG_FEATURE_P2P
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V2
		EXTRA_CFLAGS += -DCFG_FEATURE_SUPPORT_CPUDOMAIN
	else
		EXTRA_CFLAGS += -I$(PWD)/inc/soc/mini_v1/
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/devdrv/pcie_slave/soc/mini_v1/
		EXTRA_CFLAGS += -DCFG_FEATURE_PM
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V2
	endif
endif

ifeq ($(TARGET_PRODUCT),cloud)
	ifeq ($(TARGET_CHIP_ID), $(A9XXGEN02M01))
		EXTRA_CFLAGS += -I$(PWD)/inc/soc/cloud_v2/
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/devdrv/pcie_slave/soc/cloud_v2/
		EXTRA_CFLAGS += -DCFG_FEATURE_P2P
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_ESL_FPGA
	else
		EXTRA_CFLAGS += -I$(PWD)/inc/soc/cloud_v1/
		EXTRA_CFLAGS += -I$(PWD)/drivers/dev_platform/devdrv/pcie_slave/soc/cloud_v1/
		EXTRA_CFLAGS += -DCFG_FEATURE_P2P
	endif
endif

ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers

obj-m   += drv_pcie.o
drv_pcie-objs := agentdrv_pci.o
drv_pcie-objs += ctrls/agentdrv_ctrl.o ctrls/agentdrv_common_msg.o
drv_pcie-objs += ctrls/agentdrv_dfx.o ctrls/agentdrv_sysfs.o
drv_pcie-objs += unit/agentdrv_unit.o unit/devdrv_dma.o unit/devdrv_atu.o unit/devdrv_atu_interface.o
drv_pcie-objs += unit/msg/agentdrv_msg_trans.o unit/msg/agentdrv_msg_non_trans.o
drv_pcie-objs += unit/msg/agentdrv_msg.o unit/msg/agentdrv_msg_admin.o unit/msg/agentdrv_msg_p2p.o
drv_pcie-objs += unit/agentdrv_doorbell.o unit/agentdrv_mdc_p2p.o unit/agentdrv_multi_chip.o unit/agentdrv_pcie_link_info.o
drv_pcie-objs += interfaces/agentdrv_interface.o interfaces/agentdrv_msg_interface.o interfaces/devdrv_dma_interface.o
drv_pcie-objs += soc/dma_comm_drv.o soc/dma_adapt.o soc/nvme_comm_drv.o soc/apb_comm_drv.o soc/resource_comm_drv.o

ifeq ($(TARGET_PRODUCT),mini)
	ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
		drv_pcie-objs += ctrls/agentdrv_profiling.o
		drv_pcie-objs += soc/mdc_v1/agentdrv_platform.o
		drv_pcie-objs += soc/mdc_v1/dma_drv.o soc/mdc_v1/nvme_drv.o soc/mdc_v1/apb_drv.o soc/mdc_v1/resource_drv.o
	else
		drv_pcie-objs += soc/mini_v1/agentdrv_platform.o soc/mini_v1/agentdrv_linkdown.o soc/mini_v1/agentdrv_reset.o soc/mini_v1/config_secure_reg.o soc/mini_v1/agentdrv_config_secure_reg.o soc/mini_v1/agentdrv_pm.o
		drv_pcie-objs += soc/mini_v1/dma_drv.o soc/mini_v1/nvme_drv.o soc/mini_v1/apb_drv.o soc/mini_v1/resource_drv.o
	endif
endif
ifeq ($(TARGET_PRODUCT),mdc)
	ifeq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_MDC_V10))
		drv_pcie-objs += soc/mdc_v1/agentdrv_platform.o
		drv_pcie-objs += soc/mdc_v1/dma_drv.o soc/mdc_v1/nvme_drv.o soc/mdc_v1/apb_drv.o soc/mdc_v1/resource_drv.o
		drv_pcie-objs += interfaces/drv_cpu_type.o interfaces/workqueue_affinity.o
	else ifeq ($(TARGET_CHIP_ID), $(ASCEND_MDC_BS9SX1A))
		drv_pcie-objs += soc/mdc_v1/agentdrv_platform.o
		drv_pcie-objs += soc/mdc_v1/dma_drv.o soc/mdc_v1/nvme_drv.o soc/mdc_v1/apb_drv.o soc/mdc_v1/resource_drv.o
		drv_pcie-objs += interfaces/drv_cpu_type.o interfaces/workqueue_affinity.o
	else
		drv_pcie-objs += soc/mini_v1/agentdrv_platform.o soc/mini_v1/agentdrv_linkdown.o soc/mini_v1/agentdrv_reset.o soc/mini_v1/config_secure_reg.o soc/mini_v1/agentdrv_config_secure_reg.o soc/mini_v1/agentdrv_pm.o
		drv_pcie-objs += soc/mini_v1/dma_drv.o soc/mini_v1/nvme_drv.o soc/mini_v1/apb_drv.o soc/mini_v1/resource_drv.o
		drv_pcie-objs += interfaces/drv_cpu_mask.o interfaces/workqueue_affinity.o
	endif
endif
ifeq ($(TARGET_PRODUCT),cloud)
	drv_pcie-objs += ctrls/agentdrv_profiling.o
	ifeq ($(TARGET_CHIP_ID), $(A9XXGEN02M01))
		drv_pcie-objs += soc/cloud_v2/agentdrv_platform.o soc/cloud_v2/agentdrv_flr.o
		drv_pcie-objs += soc/cloud_v2/dma_drv.o soc/cloud_v2/nvme_drv.o soc/cloud_v2/apb_drv.o soc/cloud_v2/resource_drv.o
	else
		drv_pcie-objs += soc/cloud_v1/agentdrv_platform.o soc/cloud_v1/agentdrv_linkdown.o soc/cloud_v1/agentdrv_reset.o
		drv_pcie-objs += soc/cloud_v1/dma_drv.o soc/cloud_v1/nvme_drv.o soc/cloud_v1/apb_drv.o soc/cloud_v1/resource_drv.o
	endif
endif
else

EXTRA_CFLAGS += -fstack-protector-all
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver/
EXTRA_CFLAGS += -I$(TOP_DIR)/abl/bbox/inc/bbox
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/toolchain/bbox
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_pcie_device/common_h/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_pcie_device/pcie_slave/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_pcie_device/pcie_slave/unit/msg/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_pcie_device/pcie_slave/interfaces/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_pcie_device/pcie_slave/soc/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_pcie_device/pcie_slave/unit/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_pcie_device/pcie_slave/ctrls/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_pcie_device/pcie_slave/qos/

ifeq ($(ENABLE_ASAN),true)
    EXTRA_CFLAGS += -DCFG_ENABLE_ASAN
endif

ifeq ($(PRODUCT),ascend310p)
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/cloud_v1/
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_pcie_device/pcie_slave/soc/mdc_v1/
		EXTRA_CFLAGS += -DCFG_FEATURE_P2P
endif
ifneq ($(filter $(PRODUCT),  ascend310 ascend310rc),)
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/mini_v1/
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_pcie_device/pcie_slave/soc/mini_v1
        EXTRA_CFLAGS += -DCFG_FEATURE_PM -DCFG_SOC_PLATFORM_MINI
endif

ifneq ($(filter $(PRODUCT),  ascend310B ascend310Brc as31xm1),)
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/mini_v3/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_pcie_device/pcie_slave/soc/mini_v3/
endif

ifneq ($(filter $(PRODUCT),  ascend310Brc),)
EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV3_RC
endif

ifneq ($(filter $(PRODUCT), as31xm1),)
EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
EXTRA_CFLAGS += -DCFG_SOC_FEATURE_SR
EXTRA_CFLAGS += -DCFG_FEATURE_PCIE_LINK_INFO
EXTRA_CFLAGS += -DCFG_FEATURE_SUPPORT_CPUDOMAIN
endif

ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a helper310p ascend610Lite ascend610Liteesl),)
		EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/cloud_v1/
		EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_pcie_device/pcie_slave/soc/mdc_v1/
		EXTRA_CFLAGS += -DCFG_FEATURE_P2P
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V2
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
		ifneq ($(filter $(PRODUCT),  ascend610Liteesl ),)
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_LITE_ESL
		endif
		ifneq ($(filter $(PRODUCT),  ascend610Lite),)
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_LITE_DECOUPLING
		endif
		ifeq ($(filter $(PRODUCT), helper310p),)   # not helper310p
			EXTRA_CFLAGS += -DCFG_FEATURE_SUPPORT_CPUDOMAIN
		endif
endif

ifneq ($(filter $(PRODUCT),  ascend910 ),)
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/cloud_v1/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_pcie_device/pcie_slave/soc/cloud_v1/
EXTRA_CFLAGS += -DCFG_FEATURE_P2P
endif

ifneq ($(filter $(PRODUCT),  ascend910B),)
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/soc/cloud_v2/
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_pcie_device/pcie_slave/soc/cloud_v2/
EXTRA_CFLAGS += -DCFG_FEATURE_P2P
EXTRA_CFLAGS += -DCFG_FEATURE_SRIOV
EXTRA_CFLAGS += -DCFG_FEATURE_PASSID
endif

ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align
ccflags-y += -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers

obj-m   += drv_pcie.o
drv_pcie-objs := agentdrv_pci.o
drv_pcie-objs += ctrls/agentdrv_ctrl.o ctrls/agentdrv_common_msg.o
drv_pcie-objs += ctrls/agentdrv_dfx.o ctrls/agentdrv_sysfs.o
drv_pcie-objs += unit/agentdrv_unit.o unit/devdrv_dma.o unit/devdrv_atu.o unit/devdrv_atu_interface.o
drv_pcie-objs += unit/msg/agentdrv_msg_trans.o unit/msg/agentdrv_msg_non_trans.o
drv_pcie-objs += unit/msg/agentdrv_msg.o unit/msg/agentdrv_msg_admin.o unit/msg/agentdrv_msg_p2p.o
drv_pcie-objs += unit/agentdrv_doorbell.o unit/agentdrv_mdc_p2p.o unit/agentdrv_multi_chip.o unit/agentdrv_pcie_link_info.o
drv_pcie-objs += interfaces/agentdrv_interface.o interfaces/agentdrv_msg_interface.o interfaces/devdrv_dma_interface.o
drv_pcie-objs += soc/dma_comm_drv.o soc/dma_adapt.o soc/nvme_comm_drv.o soc/apb_comm_drv.o soc/resource_comm_drv.o

ifeq ($(PRODUCT),ascend310p)
	drv_pcie-objs += ctrls/agentdrv_profiling.o soc/mdc_v1/profiling_drv.o
	drv_pcie-objs += soc/mdc_v1/agentdrv_platform.o
	drv_pcie-objs += soc/mdc_v1/dma_drv.o soc/mdc_v1/nvme_drv.o soc/mdc_v1/apb_drv.o soc/mdc_v1/resource_drv.o
else  ifneq ($(filter $(PRODUCT),  ascend310 ascend310rc),)
	drv_pcie-objs += soc/mini_v1/agentdrv_platform.o soc/mini_v1/agentdrv_linkdown.o soc/mini_v1/agentdrv_reset.o soc/mini_v1/config_secure_reg.o soc/mini_v1/agentdrv_config_secure_reg.o soc/mini_v1/agentdrv_pm.o
	drv_pcie-objs += soc/mini_v1/dma_drv.o soc/mini_v1/nvme_drv.o soc/mini_v1/apb_drv.o soc/mini_v1/resource_drv.o
else ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a helper310p ascend610Lite ascend610Liteesl),)
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/common
	drv_pcie-objs += soc/mdc_v1/agentdrv_platform.o
	drv_pcie-objs += soc/mdc_v1/dma_drv.o soc/mdc_v1/nvme_drv.o soc/mdc_v1/apb_drv.o soc/mdc_v1/resource_drv.o
	ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
	    drv_pcie-objs += interfaces/drv_cpu_type.o interfaces/workqueue_affinity.o
	else
	    drv_pcie-objs += interfaces/drv_cpu_mask.o interfaces/workqueue_affinity.o
	endif
else ifneq ($(filter $(PRODUCT),  ascend910 ),)
	drv_pcie-objs += ctrls/agentdrv_profiling.o soc/cloud_v1/profiling_drv.o
	drv_pcie-objs += soc/cloud_v1/agentdrv_platform.o soc/cloud_v1/agentdrv_linkdown.o soc/cloud_v1/agentdrv_reset.o
	drv_pcie-objs += soc/cloud_v1/dma_drv.o soc/cloud_v1/nvme_drv.o soc/cloud_v1/apb_drv.o soc/cloud_v1/resource_drv.o
else ifneq ($(filter $(PRODUCT),  ascend910B ascend920esl ascend920emu ),)
	drv_pcie-objs += ctrls/agentdrv_profiling.o soc/cloud_v2/profiling_drv.o
	drv_pcie-objs += soc/cloud_v2/agentdrv_platform.o soc/cloud_v2/agentdrv_flr.o
	drv_pcie-objs += soc/cloud_v2/dma_drv.o soc/cloud_v2/nvme_drv.o soc/cloud_v2/apb_drv.o soc/cloud_v2/resource_drv.o
else ifneq ($(filter $(PRODUCT),  ascend310B ascend310Brc as31xm1),)
        drv_pcie-objs += soc/mini_v3/agentdrv_platform.o
        drv_pcie-objs += soc/mini_v3/dma_drv.o soc/mini_v3/nvme_drv.o soc/mini_v3/apb_drv.o soc/mini_v3/resource_drv.o
endif

ifneq ($(filter $(PRODUCT), ascend310B as31xm1),)
	drv_pcie-objs += qos/agentdrv_qos.o qos/agentdrv_qos_mini_v3.o qos/pcie_qos_common.o
endif

ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
	drv_pcie-objs += qos/agentdrv_qos.o qos/agentdrv_qos_cloud_v2.o qos/pcie_qos_common.o
endif

endif

