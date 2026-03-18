ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align -Wextra
ccflags-y += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-sign-compare

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dbl/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dbl/chip_config
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dbl/chip_config/numa_id

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

obj-m += dbl_chip_config.o
dbl_chip_config-objs := chip_config_module.o
dbl_chip_config-objs += numa_id/numa_id.o
dbl_chip_config-objs += numa_id/numa_id_config.o
dbl_chip_config-objs += numa_id/numa_id_proc_fs.o
