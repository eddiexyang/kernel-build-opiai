ifeq ($(TOP_DIR),)
    ifeq ($(DAVINCI_HIAI_DKMS),y)
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/inc/driver
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/libc_sec/include
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/xsmem/dc/xsmem_host
    else
        ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align -Wextra
        ccflags-y += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-sign-compare -Wno-implicit-fallthrough
        TOPDIR_ABS := $(KERNEL_DIR)
        EXTRA_CFLAGS += -I$(TOPDIR_ABS)/../../../../../inc/driver
        EXTRA_CFLAGS += -I$(TOPDIR_ABS)/../../../../../abl/libc_sec/include
        EXTRA_CFLAGS += -I$(TOPDIR_ABS)/../../../../../drivers/ai_sdk/arc/linux/kernel_space/inc
        EXTRA_CFLAGS += -I$(TOPDIR_ABS)/../../../../../drivers/ai_sdk/arc/linux/kernel_space/src/xsmem/dc/xsmem_host
    endif
else
    MAKE_PRODUCT := $(PRODUCT)

    ifeq ($(DAVINCI_HIAI_DKMS),y)
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/inc/driver
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dev_inc/inc
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/libc_sec/include
        EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/xsmem/dc/xsmem_host
    else
        ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align -Wextra
        ccflags-y += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-sign-compare -Wno-implicit-fallthrough
        EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver/
        EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/
        EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/xsmem/dc/xsmem_host
    endif
endif

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

ccflags-y += -Wall -Werror
obj-m   += ascend_xsmem.o
ascend_xsmem-objs := xsmem_framework.o xsmem_ns_adapt.o xsmem_proc_fs.o xsmem_prop.o xsmem_res_idr.o xsmem_res_dispatch.o xsmem_algo_vma.o xsmem_algo_cache_vma.o
ifeq ($(filter $(PRODUCT), helper310p),)
    EXTRA_CFLAGS += -DCFG_FEATURE_EXTERNAL_CDEV
    ascend_xsmem-objs += xsmem_modprobe_adapt.o
endif
EXTRA_CFLAGS += -DCFG_FEATURE_SUPPORT_VMA
EXTRA_CFLAGS += -DCFG_FEATURE_SUPPORT_CACHE


