ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
    EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
endif
ifeq ($(TOP_DIR),)
KERNEL_DIR :=$(srctree)
EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/abl/libc_sec/include
ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef -Wstack-usage=2048 $(WDATE_TIME)
ccflags-y += -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers
obj-m   += drv_davinci_intf.o
drv_davinci_intf-objs := davinci_intf_init.o davinci_intf_process.o
else
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a helper310p ascend610Lite ascend610Liteesl),)
    ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
        EXTRA_CFLAGS += -DCFG_FEATURE_SUPPORT_CPUDOMAIN
    endif
else
    EXTRA_CFLAGS += -DCFG_SUPPORT_OS_SVM_CLEAN
endif
    ifneq ($(filter $(PRODUCT),  ascend920esl ascend920emu),)
    else
        EXTRA_CFLAGS += -DCFG_BIND_CPU_ENABLE
    endif
ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef -Wstack-usage=2048 $(WDATE_TIME)
ccflags-y += -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers
obj-m   += drv_davinci_intf.o
drv_davinci_intf-objs := davinci_intf_init.o davinci_intf_process.o

ifneq ($(filter $(PRODUCT), ascend610 bs9sx1a ascend610Lite ascend610Liteesl),)
    drv_davinci_intf-objs += drv_cpu_type.o
else
    drv_davinci_intf-objs += drv_cpu_mask.o
endif

ifneq ($(filter $(PRODUCT), ascend610Lite ascend610Liteesl),)
    EXTRA_CFLAGS += -DCFG_SOC_MDC_V51_LITE
endif
ifneq ($(filter $(PRODUCT), ascend310Brc),)
    EXTRA_CFLAGS += -DCFG_FEATURE_MANAGE_DOCKER
endif
endif
