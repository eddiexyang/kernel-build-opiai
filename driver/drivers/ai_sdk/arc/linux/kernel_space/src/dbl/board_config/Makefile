ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align -Wextra
ccflags-y += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-sign-compare

EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/inc/dbl
EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dbl/board_config
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dbl/board_config/serdes

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

obj-m += dbl_board_config.o
dbl_board_config-objs := board_config_module.o
dbl_board_config-objs += serdes/board_serdes_info.o

